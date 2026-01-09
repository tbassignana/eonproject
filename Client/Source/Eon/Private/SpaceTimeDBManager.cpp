// Copyright 2026 Eon Project. All rights reserved.

#include "SpaceTimeDBManager.h"
#include "WebSocketsModule.h"
#include "Json.h"
#include "JsonUtilities.h"

void USpaceTimeDBManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    UE_LOG(LogTemp, Log, TEXT("SpaceTimeDBManager initialized"));
}

void USpaceTimeDBManager::Deinitialize()
{
    Disconnect();
    Super::Deinitialize();
}

void USpaceTimeDBManager::Connect(const FString& ServerUrl, const FString& DatabaseName)
{
    if (WebSocket.IsValid() && WebSocket->IsConnected())
    {
        UE_LOG(LogTemp, Warning, TEXT("Already connected to SpaceTimeDB"));
        return;
    }

    if (bIsConnecting)
    {
        UE_LOG(LogTemp, Warning, TEXT("Connection already in progress"));
        return;
    }

    bIsConnecting = true;

    // Construct WebSocket URL for SpaceTimeDB
    // Format: wss://<server>/database/websocket/<database_name>
    FString WsUrl = ServerUrl;
    if (!WsUrl.StartsWith(TEXT("ws://")) && !WsUrl.StartsWith(TEXT("wss://")))
    {
        WsUrl = TEXT("wss://") + WsUrl;
    }
    WsUrl = WsUrl + TEXT("/database/websocket/") + DatabaseName;

    UE_LOG(LogTemp, Log, TEXT("Connecting to SpaceTimeDB: %s"), *WsUrl);

    // Create WebSocket
    WebSocket = FWebSocketsModule::Get().CreateWebSocket(WsUrl, TEXT(""));

    // Bind callbacks
    WebSocket->OnConnected().AddUObject(this, &USpaceTimeDBManager::HandleWebSocketConnected);
    WebSocket->OnConnectionError().AddUObject(this, &USpaceTimeDBManager::HandleWebSocketConnectionError);
    WebSocket->OnClosed().AddUObject(this, &USpaceTimeDBManager::HandleWebSocketClosed);
    WebSocket->OnMessage().AddUObject(this, &USpaceTimeDBManager::HandleWebSocketMessage);

    // Initiate connection
    WebSocket->Connect();
}

void USpaceTimeDBManager::Disconnect()
{
    if (WebSocket.IsValid())
    {
        if (WebSocket->IsConnected())
        {
            WebSocket->Close();
        }
        WebSocket.Reset();
    }

    Identity = TEXT("");
    Token = TEXT("");
    PendingReducerCalls.Empty();
    ActiveSubscriptions.Empty();
    bIsConnecting = false;
}

bool USpaceTimeDBManager::IsConnected() const
{
    return WebSocket.IsValid() && WebSocket->IsConnected();
}

void USpaceTimeDBManager::CallReducer(const FString& ReducerName, const TArray<FString>& Args)
{
    if (!IsConnected())
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot call reducer - not connected"));
        return;
    }

    // Build reducer call message
    TSharedPtr<FJsonObject> Message = MakeShareable(new FJsonObject());
    Message->SetStringField(TEXT("type"), TEXT("call"));

    TSharedPtr<FJsonObject> CallData = MakeShareable(new FJsonObject());
    CallData->SetStringField(TEXT("reducer"), ReducerName);

    TArray<TSharedPtr<FJsonValue>> ArgsArray;
    for (const FString& Arg : Args)
    {
        ArgsArray.Add(MakeShareable(new FJsonValueString(Arg)));
    }
    CallData->SetArrayField(TEXT("args"), ArgsArray);

    uint32 RequestId = GenerateRequestId();
    CallData->SetNumberField(TEXT("request_id"), RequestId);

    Message->SetObjectField(TEXT("call"), CallData);

    PendingReducerCalls.Add(RequestId, ReducerName);
    SendMessage(Message);

    UE_LOG(LogTemp, Log, TEXT("Called reducer: %s (request_id: %d)"), *ReducerName, RequestId);
}

void USpaceTimeDBManager::Subscribe(const FString& TableName)
{
    if (!IsConnected())
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot subscribe - not connected"));
        return;
    }

    if (ActiveSubscriptions.Contains(TableName))
    {
        UE_LOG(LogTemp, Log, TEXT("Already subscribed to table: %s"), *TableName);
        return;
    }

    // Build subscription message
    TSharedPtr<FJsonObject> Message = MakeShareable(new FJsonObject());
    Message->SetStringField(TEXT("type"), TEXT("subscribe"));

    TSharedPtr<FJsonObject> SubData = MakeShareable(new FJsonObject());

    // Subscribe with SQL query
    TArray<TSharedPtr<FJsonValue>> QueriesArray;
    FString Query = FString::Printf(TEXT("SELECT * FROM %s"), *TableName);
    QueriesArray.Add(MakeShareable(new FJsonValueString(Query)));
    SubData->SetArrayField(TEXT("queries"), QueriesArray);

    Message->SetObjectField(TEXT("subscribe"), SubData);

    SendMessage(Message);
    ActiveSubscriptions.Add(TableName);

    UE_LOG(LogTemp, Log, TEXT("Subscribed to table: %s"), *TableName);
}

void USpaceTimeDBManager::Unsubscribe(const FString& TableName)
{
    if (!ActiveSubscriptions.Contains(TableName))
    {
        return;
    }

    // TODO: Implement unsubscribe message
    ActiveSubscriptions.Remove(TableName);
    UE_LOG(LogTemp, Log, TEXT("Unsubscribed from table: %s"), *TableName);
}

void USpaceTimeDBManager::HandleWebSocketConnected()
{
    bIsConnecting = false;
    UE_LOG(LogTemp, Log, TEXT("Connected to SpaceTimeDB"));

    // The server will send an IdentityToken message
    // We wait for that before broadcasting OnConnected
}

void USpaceTimeDBManager::HandleWebSocketConnectionError(const FString& Error)
{
    bIsConnecting = false;
    UE_LOG(LogTemp, Error, TEXT("SpaceTimeDB connection error: %s"), *Error);
    OnError.Broadcast(Error, false);
}

void USpaceTimeDBManager::HandleWebSocketClosed(int32 StatusCode, const FString& Reason, bool WasClean)
{
    bIsConnecting = false;
    UE_LOG(LogTemp, Log, TEXT("SpaceTimeDB connection closed: %s (code: %d, clean: %d)"),
           *Reason, StatusCode, WasClean);

    Identity = TEXT("");
    Token = TEXT("");
    ActiveSubscriptions.Empty();

    OnDisconnected.Broadcast(Reason);
}

void USpaceTimeDBManager::HandleWebSocketMessage(const FString& Message)
{
    TSharedPtr<FJsonObject> JsonMessage;
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Message);

    if (!FJsonSerializer::Deserialize(Reader, JsonMessage) || !JsonMessage.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("Failed to parse SpaceTimeDB message"));
        return;
    }

    FString MessageType;
    if (!JsonMessage->TryGetStringField(TEXT("type"), MessageType))
    {
        // Try alternative message format
        if (JsonMessage->HasField(TEXT("IdentityToken")))
        {
            HandleIdentityToken(JsonMessage->GetObjectField(TEXT("IdentityToken")));
            return;
        }
        if (JsonMessage->HasField(TEXT("SubscriptionUpdate")))
        {
            HandleSubscriptionUpdate(JsonMessage->GetObjectField(TEXT("SubscriptionUpdate")));
            return;
        }
        if (JsonMessage->HasField(TEXT("TransactionUpdate")))
        {
            HandleTransactionUpdate(JsonMessage->GetObjectField(TEXT("TransactionUpdate")));
            return;
        }
        if (JsonMessage->HasField(TEXT("OneOffQueryResponse")))
        {
            HandleReducerResponse(JsonMessage->GetObjectField(TEXT("OneOffQueryResponse")));
            return;
        }

        UE_LOG(LogTemp, Warning, TEXT("Unknown SpaceTimeDB message format"));
        return;
    }

    // Handle typed messages
    if (MessageType == TEXT("identity_token"))
    {
        HandleIdentityToken(JsonMessage);
    }
    else if (MessageType == TEXT("subscription_update"))
    {
        HandleSubscriptionUpdate(JsonMessage);
    }
    else if (MessageType == TEXT("transaction_update"))
    {
        HandleTransactionUpdate(JsonMessage);
    }
    else if (MessageType == TEXT("call_response"))
    {
        HandleReducerResponse(JsonMessage);
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("Received SpaceTimeDB message type: %s"), *MessageType);
    }
}

void USpaceTimeDBManager::HandleIdentityToken(const TSharedPtr<FJsonObject>& Message)
{
    if (Message->TryGetStringField(TEXT("identity"), Identity))
    {
        Message->TryGetStringField(TEXT("token"), Token);
        UE_LOG(LogTemp, Log, TEXT("Received identity: %s"), *Identity);

        // Now we're fully connected
        OnConnected.Broadcast();
    }
}

void USpaceTimeDBManager::HandleSubscriptionUpdate(const TSharedPtr<FJsonObject>& Message)
{
    // TODO: Parse row updates and broadcast to listeners
    // This contains initial table data and real-time updates

    UE_LOG(LogTemp, Verbose, TEXT("Received subscription update"));

    // The message contains:
    // - table_updates: array of { table_name, rows: [ { op: "insert"|"delete", row: {...} } ] }
    // We need to parse these and update local state
}

void USpaceTimeDBManager::HandleTransactionUpdate(const TSharedPtr<FJsonObject>& Message)
{
    // Transaction updates contain results of reducer calls
    // and any table changes that resulted from them

    UE_LOG(LogTemp, Verbose, TEXT("Received transaction update"));
}

void USpaceTimeDBManager::HandleReducerResponse(const TSharedPtr<FJsonObject>& Message)
{
    int32 RequestId = 0;
    if (Message->TryGetNumberField(TEXT("request_id"), RequestId))
    {
        FString* ReducerName = PendingReducerCalls.Find(RequestId);
        if (ReducerName)
        {
            UE_LOG(LogTemp, Log, TEXT("Reducer %s completed (request_id: %d)"), **ReducerName, RequestId);
            PendingReducerCalls.Remove(RequestId);
        }
    }
}

void USpaceTimeDBManager::SendMessage(const TSharedPtr<FJsonObject>& Message)
{
    if (!IsConnected())
    {
        return;
    }

    FString JsonString;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonString);
    FJsonSerializer::Serialize(Message.ToSharedRef(), Writer);

    WebSocket->Send(JsonString);
}

uint32 USpaceTimeDBManager::GenerateRequestId()
{
    return NextRequestId++;
}
