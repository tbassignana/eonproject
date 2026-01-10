// Copyright 2026 tbassignana. MIT License.

#include "SpaceTimeDBManager.h"
#include "WebSocketsModule.h"
#include "Json.h"
#include "JsonUtilities.h"
#include "TimerManager.h"
#include "Engine/World.h"

void USpaceTimeDBManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	FModuleManager::Get().LoadModuleChecked(TEXT("WebSockets"));
}

void USpaceTimeDBManager::Deinitialize()
{
	Disconnect();
	Super::Deinitialize();
}

void USpaceTimeDBManager::Connect(const FSpaceTimeDBConfig& Config)
{
	CurrentConfig = Config;
	ReconnectAttempts = 0;

	if (WebSocket.IsValid() && WebSocket->IsConnected())
	{
		Disconnect();
	}

	FString Url = FString::Printf(TEXT("%s/database/subscribe/%s"),
		*CurrentConfig.Host, *CurrentConfig.ModuleName);

	WebSocket = FWebSocketsModule::Get().CreateWebSocket(Url, TEXT("wss"));

	WebSocket->OnConnected().AddLambda([this]()
	{
		bIsConnected = true;
		ReconnectAttempts = 0;
		UE_LOG(LogTemp, Log, TEXT("SpaceTimeDB: Connected"));
		OnConnected.Broadcast();

		// Subscribe to relevant tables
		Subscribe(TEXT("SELECT * FROM player"));
		Subscribe(TEXT("SELECT * FROM instance WHERE is_public = true"));
		Subscribe(TEXT("SELECT * FROM inventory_item"));
		Subscribe(TEXT("SELECT * FROM world_item"));
		Subscribe(TEXT("SELECT * FROM interactable_state"));
	});

	WebSocket->OnConnectionError().AddLambda([this](const FString& Error)
	{
		UE_LOG(LogTemp, Error, TEXT("SpaceTimeDB: Connection error - %s"), *Error);
		bIsConnected = false;
		AttemptReconnect();
	});

	WebSocket->OnClosed().AddLambda([this](int32 StatusCode, const FString& Reason, bool bWasClean)
	{
		UE_LOG(LogTemp, Warning, TEXT("SpaceTimeDB: Disconnected - %s"), *Reason);
		bIsConnected = false;
		OnDisconnected.Broadcast(Reason);

		if (!bWasClean)
		{
			AttemptReconnect();
		}
	});

	WebSocket->OnMessage().AddLambda([this](const FString& Message)
	{
		HandleMessage(Message);
	});

	WebSocket->Connect();
}

void USpaceTimeDBManager::Disconnect()
{
	if (WebSocket.IsValid())
	{
		WebSocket->Close();
		WebSocket.Reset();
	}
	bIsConnected = false;

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(ReconnectTimerHandle);
	}
}

bool USpaceTimeDBManager::IsConnected() const
{
	return bIsConnected && WebSocket.IsValid() && WebSocket->IsConnected();
}

void USpaceTimeDBManager::AttemptReconnect()
{
	if (ReconnectAttempts >= CurrentConfig.MaxReconnectAttempts)
	{
		UE_LOG(LogTemp, Error, TEXT("SpaceTimeDB: Max reconnect attempts reached"));
		return;
	}

	ReconnectAttempts++;
	UE_LOG(LogTemp, Log, TEXT("SpaceTimeDB: Reconnect attempt %d/%d"),
		ReconnectAttempts, CurrentConfig.MaxReconnectAttempts);

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			ReconnectTimerHandle,
			[this]() { Connect(CurrentConfig); },
			CurrentConfig.ReconnectDelay,
			false
		);
	}
}

void USpaceTimeDBManager::CallReducer(const FString& ReducerName, const TArray<FString>& Args)
{
	if (!IsConnected())
	{
		UE_LOG(LogTemp, Warning, TEXT("SpaceTimeDB: Not connected, cannot call reducer"));
		return;
	}

	TSharedPtr<FJsonObject> CallObj = MakeShareable(new FJsonObject);
	CallObj->SetStringField(TEXT("call"), ReducerName);

	TArray<TSharedPtr<FJsonValue>> ArgsArray;
	for (const FString& Arg : Args)
	{
		ArgsArray.Add(MakeShareable(new FJsonValueString(Arg)));
	}
	CallObj->SetArrayField(TEXT("args"), ArgsArray);

	FString OutputString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
	FJsonSerializer::Serialize(CallObj.ToSharedRef(), Writer);

	WebSocket->Send(OutputString);
}

void USpaceTimeDBManager::Subscribe(const FString& Query)
{
	if (!IsConnected())
	{
		return;
	}

	TSharedPtr<FJsonObject> SubObj = MakeShareable(new FJsonObject);
	SubObj->SetStringField(TEXT("subscribe"), Query);

	FString OutputString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
	FJsonSerializer::Serialize(SubObj.ToSharedRef(), Writer);

	WebSocket->Send(OutputString);
}

void USpaceTimeDBManager::HandleMessage(const FString& Message)
{
	TSharedPtr<FJsonObject> JsonObject;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Message);

	if (!FJsonSerializer::Deserialize(Reader, JsonObject) || !JsonObject.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("SpaceTimeDB: Failed to parse message"));
		return;
	}

	FString MessageType;
	if (JsonObject->TryGetStringField(TEXT("type"), MessageType))
	{
		if (MessageType == TEXT("TransactionUpdate"))
		{
			// Handle row updates
			const TArray<TSharedPtr<FJsonValue>>* Updates;
			if (JsonObject->TryGetArrayField(TEXT("updates"), Updates))
			{
				for (const auto& Update : *Updates)
				{
					const TSharedPtr<FJsonObject>* UpdateObj;
					if (Update->TryGetObject(UpdateObj))
					{
						FString TableName;
						(*UpdateObj)->TryGetStringField(TEXT("table"), TableName);

						if (TableName == TEXT("player"))
						{
							FString PlayerId, Data;
							(*UpdateObj)->TryGetStringField(TEXT("identity"), PlayerId);

							TSharedRef<TJsonWriter<>> DataWriter = TJsonWriterFactory<>::Create(&Data);
							FJsonSerializer::Serialize(UpdateObj->ToSharedRef(), DataWriter);

							OnPlayerDataReceived.Broadcast(PlayerId, Data);
						}
						else if (TableName == TEXT("inventory_item"))
						{
							FString Data;
							TSharedRef<TJsonWriter<>> DataWriter = TJsonWriterFactory<>::Create(&Data);
							FJsonSerializer::Serialize(UpdateObj->ToSharedRef(), DataWriter);
							OnInventoryUpdated.Broadcast(Data);
						}
					}
				}
			}
		}
		else if (MessageType == TEXT("IdentityToken"))
		{
			JsonObject->TryGetStringField(TEXT("identity"), Identity);
			UE_LOG(LogTemp, Log, TEXT("SpaceTimeDB: Identity set - %s"), *Identity);
		}
	}
}

// Player Management
void USpaceTimeDBManager::RegisterPlayer(const FString& Username)
{
	CallReducer(TEXT("register_player"), { Username });
}

void USpaceTimeDBManager::UpdatePlayerPosition(FVector Position, FRotator Rotation)
{
	CallReducer(TEXT("update_player_position"), {
		FString::SanitizeFloat(Position.X),
		FString::SanitizeFloat(Position.Y),
		FString::SanitizeFloat(Position.Z),
		FString::SanitizeFloat(Rotation.Pitch),
		FString::SanitizeFloat(Rotation.Yaw),
		FString::SanitizeFloat(Rotation.Roll)
	});
}

void USpaceTimeDBManager::SetPlayerOnline(bool bOnline)
{
	CallReducer(TEXT("set_player_online"), { bOnline ? TEXT("true") : TEXT("false") });
}

// Instance Management
void USpaceTimeDBManager::CreateInstance(const FString& Name, int32 MaxPlayers, bool bIsPublic)
{
	CallReducer(TEXT("create_instance"), {
		Name,
		FString::FromInt(MaxPlayers),
		bIsPublic ? TEXT("true") : TEXT("false")
	});
}

void USpaceTimeDBManager::JoinInstance(int64 InstanceId)
{
	CallReducer(TEXT("join_instance"), { FString::Printf(TEXT("%lld"), InstanceId) });
}

void USpaceTimeDBManager::LeaveInstance()
{
	CallReducer(TEXT("leave_instance"), {});
}

void USpaceTimeDBManager::RequestInstanceList()
{
	Subscribe(TEXT("SELECT * FROM instance WHERE is_public = true"));
}

// Inventory Management
void USpaceTimeDBManager::AddItemToInventory(const FString& ItemId, int32 Quantity)
{
	CallReducer(TEXT("add_item_to_inventory"), {
		ItemId,
		FString::FromInt(Quantity)
	});
}

void USpaceTimeDBManager::RemoveItemFromInventory(int64 EntryId, int32 Quantity)
{
	CallReducer(TEXT("remove_item_from_inventory"), {
		FString::Printf(TEXT("%lld"), EntryId),
		FString::FromInt(Quantity)
	});
}

void USpaceTimeDBManager::UseConsumable(int64 EntryId)
{
	CallReducer(TEXT("use_consumable"), { FString::Printf(TEXT("%lld"), EntryId) });
}

void USpaceTimeDBManager::CollectWorldItem(int64 WorldItemId)
{
	CallReducer(TEXT("collect_world_item"), { FString::Printf(TEXT("%lld"), WorldItemId) });
}

// Interactables
void USpaceTimeDBManager::ToggleInteractable(const FString& InteractableId)
{
	CallReducer(TEXT("toggle_interactable"), { InteractableId });
}
