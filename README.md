# Interactive Object Manager Demo

IOManager is a small Unreal Engine 5.6 C++ project that lets the user spawn, select and modify simple objects in a level.  
The project is built for a senior Unreal Engine C++ technical assessment and focuses on clean structure and clear data flow between C++, UI and configuration.

The assessment time budget is 24 hours. The actual focused implementation time for this demo was about 16 hours, with priority on architecture and code quality rather than extra features.

---

## 1. Project overview

In the running app the player can:

- spawn new objects in the level using current default settings
- see all spawned objects in a list
- select an object from the list
- change the selected object's color and scale
- delete the selected object
- open a Settings tab to view and edit default spawn type, color and scale

Default values are stored in an ini file. When the user changes them and saves, all new objects use the updated defaults.

### Demo level and controls

- Demo level: `L_InteractiveObjectDemo_Basic`

- Camera movement:
  - `W`, `A`, `S`, `D` move the camera on the horizontal plane
  - `E` and `Q` move the camera up and down

- Mouse:
  - hold the right mouse button to move the camera and look around
  - release the right mouse button to return the cursor and interact with the UI panel

- Exit:
  - press `Esc` to exit the game build

---

## 2. Setup instructions

This section describes how to configure and run the demo in a clean project environment.

1. Open the project with Unreal Engine 5.6.
2. Open **Edit → Project Settings → Game → Interactive Object Manager**.
3. In **Primitive Classes**:
   - set **Cube Primitive Class** to `BP_InteractiveCube` under `/Game/InteractiveObjectManager/Actors`
   - set **Sphere Primitive Class** to `BP_InteractiveSphere` under the same folder
4. In **Spawn Settings**:
   - adjust **Spawn Radius** to define how far from the world origin new objects can appear on the X and Y axes
   - adjust **Spawn Height** to define the Z offset for spawned objects
5. Open the level `L_InteractiveObjectDemo_Basic`.
6. Press **Play in Editor** or run the packaged build.
7. Use the camera and mouse controls from the overview section and interact with the **Interactive Object Manager** panel to test spawning and editing objects.

The runtime defaults for spawn type, color and scale are loaded from the `[InteractiveObjectManager.Settings]` section and can be edited at runtime through the Settings tab.

---

## 3. Technology

- Engine: Unreal Engine 5.6
- Language: C++
- UI: UMG widgets; the root panel class derives from `UCommonActivatableWidget`, but individual controls in this version are standard UMG widgets
- Config: Unreal config system (GConfig, `[InteractiveObjectManager.Settings]` in `DefaultGame.ini` via `GGameIni`)
- Target platform: Windows desktop
- Logs: a CommonUI viewport validation check is disabled in project config so that runtime logs stay clean and free of non critical warnings

---

## 4. High level architecture

The project is intentionally small and is split into three main parts.

- Game project `IOManager`  
  Standard UE 5.6 C++ project that owns maps and startup settings and loads the demo level.

- Custom C++ module `InteractiveObjectManager`  
  Owns the logic for:
  - tracking spawned interactive objects
  - exposing functions for spawn, delete, selection and editing
  - providing Blueprint friendly APIs for the UI
  - reading and writing runtime defaults through a dedicated settings object

- Settings and UI  
  - a C++ settings class that reads and writes default values to a custom section in `DefaultGame.ini` using the Unreal config system  
  - a UMG screen driven by a root widget that inherits from `UCommonActivatableWidget`; in this version tabs and buttons are implemented with standard UMG controls

Most gameplay logic lives in C++. Blueprints are used mainly for UI binding and simple presentation.

---

## 5. Design choices

- Composition instead of a single base actor  
  Any actor can become interactive by adding a dedicated component instead of inheriting from a special base class. This keeps the system flexible and non intrusive.

- Clear API between C++ and UI  
  The UI calls a small set of Blueprint callable functions on the root widget and subsystem instead of touching game objects directly. All state changes go through C++.

- Event driven updates  
  The UI is updated when objects or settings change, not on Tick. The world subsystem broadcasts events for list and selection changes and the UI listens to those events.

- Config through a single settings wrapper  
  All ini access goes through one settings class that handles parsing, defaults, validation and logging. Runtime settings are held in a snapshot structure protected by a critical section.

- Focused scope within the time budget  
  With about 16 hours of focused implementation time, the effort went into a clean module layout and clear data flow rather than extra visuals or additional gameplay features.

---

## Making any Actor interactive

Any `AActor` can become interactive in this demo by adding the interactive object component provided by the module.

### What the interactive component does

The interactive component is responsible for visual control and registration of the Actor:

- stores the current color of the interactive object
- stores the current uniform scale
- locates the target `UStaticMeshComponent` on the owner Actor or uses an explicitly set target component
- creates dynamic material instances for mesh materials when the color is changed the first time
- applies color changes to the material using a configurable color parameter name
- applies uniform scale changes to the mesh or the Actor, depending on configuration
- automatically registers and unregisters itself in the manager subsystem so that the Actor appears in the UI list and can be selected and modified

The component does not use Tick. All changes are applied only when its methods are called explicitly from the subsystem or other systems.

### Example usage in this demo

The demo project includes simple examples:

- `BP_InteractiveCube`
- `BP_InteractiveSphere`

Each blueprint:

- contains a `StaticMeshComponent` with a material that exposes a color parameter  
- has the interactive component attached  
- is used by the manager subsystem when spawning new objects

When a new object is spawned, the subsystem:

- chooses which primitive to spawn based on the current default spawn type
- spawns the configured cube or sphere class
- locates its interactive component
- applies default color and scale from the settings

To make any other Blueprint interactive:

1. Add a `StaticMeshComponent` with a material that has a vector color parameter.
2. Add the interactive component from this module.
3. Optionally adjust in Details:
   - initial color
   - initial uniform scale
   - color parameter name
   - target component for scale if it should be applied to a specific mesh
4. The component will register itself in the world subsystem on BeginPlay.
5. The subsystem and UI will treat the actor like any other interactive primitive.

---

## Using the Interactive Object Manager

### Initial configuration

Before running the demo for the first time, configure the module in Project Settings:

1. Open **Edit → Project Settings → Game → Interactive Object Manager**.
2. In the **Primitive Classes** section:
   - set **Cube Primitive Class** to your cube blueprint, for example `BP_InteractiveCube` under `/Game/InteractiveObjectManager/Actors`
   - set **Sphere Primitive Class** to your sphere blueprint, for example `BP_InteractiveSphere` under the same folder
3. In the **Spawn Settings** section:
   - adjust **Spawn Radius** to control how far from the world origin new objects can appear on the X/Y plane
   - adjust **Spawn Height** to control the Z offset for spawned objects
4. Runtime defaults for spawn type, color and scale are stored in the ini section

   `[InteractiveObjectManager.Settings]`

   in `DefaultGame.ini` and are managed by the settings class.

   - default spawn type drives which primitive is used by the Spawn button
   - default color and default scale are applied to a newly spawned object through the interactive component

### Running the demo

1. Open the demo level `L_InteractiveObjectDemo_Basic`.
2. Press **Play in Editor**.
3. Use the camera and mouse controls described in the project overview.
4. Interact with the **Interactive Object Manager** panel to spawn and manage objects.

You should see the manager panel with two tabs: Main and Settings.

### Main tab

The Main tab is connected to the manager subsystem and allows you to work with all interactive objects in the current world.

- **Spawn object**  
  Calls the spawn function on the subsystem.  
  - The actual primitive class (cube or sphere) is chosen based on the current default spawn type from the settings.
  - Default color and scale are taken from the ini backed settings and applied through the interactive component.

- **Objects list**  
  - shows all actors that have the interactive component attached and are registered in the subsystem
  - selecting a row changes the current selection in the subsystem and updates the selected object label in the UI

- **Scale controls**  
  - enter a numeric value in the scale field and press **Apply scale**
  - the subsystem updates uniform scale on the currently selected object via its interactive component

- **Color controls**  
  - enter values for R, G and B and press **Apply color**
  - the subsystem updates the color on the selected object and the interactive component refreshes the dynamic material instance

- **Delete selected object**  
  - removes the currently selected actor from the world, unregisters it from the subsystem and updates the UI list
  - if there are other objects left, one of them becomes the new selection

### Settings tab

The Settings tab is connected to the settings class and works with the ini backed defaults.

On opening the Settings tab the root widget:

- queries the settings class for a snapshot of current defaults
- fills UI controls with:
  - default spawn type in a combo box (Cube, Sphere or Random)
  - default color as three numeric fields for R, G and B
  - default uniform scale as a numeric field

Two buttons control how changes are applied:

- **Apply**  
  - reads values from the UI controls  
  - sends them to the settings class as a view data structure  
  - the settings class validates and clamps values as needed  
  - runtime defaults are updated in memory  
  - new spawns use the updated defaults immediately, the ini file is not written yet

- **Save**  
  - performs the same runtime update as Apply  
  - writes validated defaults to the `[InteractiveObjectManager.Settings]` section in `DefaultGame.ini` using GConfig  
  - after restarting the game the new defaults are loaded from ini and shown again in the Settings tab

If invalid or corrupted values are found in the config file, the settings class logs a warning and falls back to safe defaults so the demo continues to run without hard failures.

---

## 6. Known limitations and future improvements

The current implementation focuses on satisfying the assessment requirements with clean architecture. Some optional features that could be added in future iterations:

- **List selection feedback in the world**  
  The selected object is reflected in the UI label and list selection, but there is no visual highlight or outline on the object in the level.  
  A natural extension would be to add an optional highlight component or post process effect for the currently selected actor.

- **Selection directly in the level**  
  At the moment selection is driven only from the UI list.  
  Another possible improvement is to allow selecting objects by clicking them in the world using a trace from the camera and then updating the selection in the manager subsystem and UI.

- **Object transform editing**  
  The demo supports color and uniform scale editing as requested by the task.  
  It does not provide position or rotation controls. A future version could expose translation and rotation gizmos or numeric fields and route those changes through the same manager subsystem.

- **Per object persistence**  
  Only default settings are persisted between runs. Individual spawned objects and their edited properties are not saved.  
  A more advanced version could serialize the list of objects and restore them on level load.

- **Partial use of CommonUI**  
  The root widget uses `UCommonActivatableWidget` for lifecycle and input mode control, but the rest of the hierarchy is built with standard UMG widgets.  
  A future iteration could fully migrate the Main and Settings tabs to CommonUI specific widgets and patterns to better demonstrate CommonUI workflows.

These limitations are intentional for the scope and time budget of the assessment and leave clear room for future extensions if needed.

---