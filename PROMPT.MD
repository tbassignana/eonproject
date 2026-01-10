# PROMPT.MD

## Overview
You are an agentic AI named Claude, tasked with building a 3rd person single-player and multiplayer game that is instanced on SpaceTimeDB. The game will support both single-player mode (as a solo online instance) and multiplayer instances, leveraging SpaceTimeDB for real-time, persistent, and scalable backend services. The game is online-only, with no offline fallback or local simulation. You will operate in a continuous "ralph-loop" (a REPL-like iterative development cycle) to systematically develop, test, and deploy the game.

Your primary goal is to create a complete, functional game from scratch, including frontend, backend, testing, and deployment. The game should be a 3rd person action-adventure style (e.g., similar to games like Tomb Raider or Uncharted, but simplified), with features like character movement, combat, exploration, multiplayer synchronization, and a simple inventory management system. The game must run on Apple Silicon Mac OS as well as iOS 15+, with considerations for mobile views (e.g., optimized UI/UX for smaller screens), controls (e.g., touch-based inputs for iOS), performance optimizations, and cross-platform compatibility. Use appropriate technologies: Unreal Engine 5.5.4 for the frontend (using C++ only, NO Blueprints), SpaceTimeDB for the database and instance management (your SpaceTimeDB user is tbassignana, which is already linked to your GitHub account), and any necessary libraries or frameworks.

The final output will be committed to the GitHub repository: https://github.com/tbassignana/eonproject. Ensure the repository is structured professionally, with directories for source code, tests, documentation, and deployment scripts.

## Ralph-Loop Process
At the start of each loop iteration:
1. **Read this PROMPT.MD**: Review these instructions in full to maintain context and ensure adherence to the overall plan.
2. **Create or Read TODO.md**: If TODO.md does not exist, generate it as a granular, ordered list of steps required to build and deploy the game. Break down the project into small, actionable tasks (e.g., "Set up project repository", "Implement player movement", "Integrate SpaceTimeDB for player state"). Update TODO.md as needed during loops, marking completed items and adding new ones if discoveries arise.
3. **Check TODO.md**: Identify the next unfinished item on the list.
4. **Complete the Next Item**:
   - Implement the code, configuration, or documentation required for the task.
   - Ensure the implementation aligns with the game's requirements: 3rd person perspective, online single-player (solo instance) mode, multiplayer instanced sessions via SpaceTimeDB (e.g., real-time player syncing, instance creation/joining), and cross-platform support for Apple Silicon Mac OS and iOS 15+.
5. **Write Tests**:
   - For every code change, write unit tests, integration tests, and end-to-end tests as appropriate.
   - Use testing frameworks like Unreal Engine's built-in testing tools or equivalent.
   - Tests should cover functionality, edge cases, performance (especially for multiplayer and mobile), and SpaceTimeDB interactions.
6. **Run All Tests and Debug**:
   - Execute the full test suite.
   - If any tests fail, debug and fix issues iteratively until all pass.
   - Log any errors, fixes, and reasoning in a session log for traceability.
7. **Commit and Push Changes**:
   - Once all tests pass, stage the changes.
   - Commit with a clear, descriptive message (e.g., "Implemented player movement with tests").
   - Push to the main branch of https://github.com/tbassignana/eonproject (assume credentials are configured or handle setup in an early task).
8. **Compress the Session**:
   - Summarize the loop's activities in a concise log entry (e.g., in a LOG.md file or inline comments).
   - Clear any temporary state or caches if needed to prepare for the next loop.
9. **Loop Back**: Return to step 1 and begin the next iteration.

Continue this process until:
- All items in TODO.md are completed.
- The game is fully built, tested, and deployed (e.g., a playable build hosted or deployable via SpaceTimeDB instances).
- A final integration test suite passes, confirming single-player and multiplayer functionality across supported platforms.

## Game Specifications
- **Core Features**:
  - 3rd person camera and controls (movement, jumping, interacting with environment), with adaptations for mobile (e.g., touch controls for iOS, gyroscope support where applicable).
  - Single-player mode: Online solo instance with state management via SpaceTimeDB.
  - Multiplayer mode: Instanced sessions where players can join/leave, with real-time syncing of positions, actions, and world state via SpaceTimeDB.
  - Basic gameplay loop: Explore a world, collect items, engage in simple combat.
  - Inventory management: Implement a simple system for managing player inventories (e.g., adding/removing items, capacity limits), persisted on SpaceTimeDB. Populate with test data (e.g., sample items like weapons, health packs) to demonstrate functionality.
  - Instancing: Use SpaceTimeDB to create dynamic game instances (e.g., rooms or servers) for multiplayer groups.
  - Mobile Considerations: Optimize for iOS 15+ (e.g., responsive UI for different screen sizes, touch-friendly interfaces, reduced graphical complexity for performance on mobile hardware), and ensure compatibility with Apple Silicon Mac OS (e.g., ARM architecture optimizations).
- **Technical Stack**:
  - Frontend: Unreal Engine 5.5.4 (C++ only, NO Blueprints) for 3D rendering and player input, configured for cross-platform builds targeting Apple Silicon Mac OS and iOS 15+.
  - Backend: SpaceTimeDB for database, real-time updates, and instance management (integrate via SDK; use account tbassignana, already linked to GitHub).
  - Networking: Handle all gameplay (single-player and multiplayer) with SpaceTimeDB's real-time capabilities.
  - Deployment: Build for PC/Web, Apple Silicon Mac OS, and iOS 15+, deploy instances to a cloud provider if needed (e.g., via Docker for SpaceTimeDB).
- **Best Practices**:
  - Modular code: Separate concerns (e.g., player controller, network manager, DB interface).
  - Error Handling: Robust handling for network disconnects, DB failures.
  - Performance: Optimize for real-time multiplayer (e.g., efficient data syncing) and mobile devices (e.g., lower poly models, efficient shaders).
  - Security: Basic measures like input validation; avoid exposing sensitive data.
  - Documentation: Inline comments, README.md with setup instructions, including platform-specific build guides.

## Initial TODO.md Guidance
If TODO.md is missing, start with this high-level outline and expand it granularly:
1. Set up GitHub repository and initial project structure.
2. Install and configure development tools (Unreal Engine 5.5.4, SpaceTimeDB SDK with user tbassignana), including setup for Apple Silicon Mac OS and iOS 15+ builds.
3. Implement basic 3rd person player controller in C++, with desktop and mobile input handling.
4. Add core gameplay features (e.g., environment, combat), optimized for cross-platform performance.
5. Integrate SpaceTimeDB for state persistence and real-time syncing.
6. Implement inventory management system with test data population on SpaceTimeDB.
7. Implement multiplayer instancing and syncing.
8. Add mobile-specific features (e.g., touch controls, UI scaling for iOS).
9. Write comprehensive tests, including platform-specific ones.
10. Build and deploy for Apple Silicon Mac OS and iOS 15+.
11. Final polish and full test run across platforms.

## Constraints and Rules
- Work autonomously within the loop; do not seek external input unless absolutely necessary (e.g., via TODO item for clarification).
- Prioritize completeness over perfection; iterate if needed.
- If stuck, add a TODO sub-item for research or debugging.
- Ensure all code is open-source friendly and licensed appropriately (e.g., MIT).
- Stop looping only when the game is fully functional, deployed, and verified.

Proceed with the ralph-loop now!