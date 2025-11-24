# Interactive Object Manager Demo

IOManager is a small Unreal Engine 5.6 C++ project that lets the user spawn, select and modify simple objects in a level.  
The project is built for a senior Unreal Engine C++ technical assessment and focuses on clean structure and clear data flow between C++, UI and configuration.

---

## 1. Project overview

In the running app the player can:

- spawn new objects in the level (at least cubes and spheres)
- see all spawned objects in a list
- select an object from the list
- change the selected object's color and scale
- delete the selected object
- open a Settings tab to view and edit default spawn type, color and scale

Default values are stored in an ini file. When the user changes them and saves, all new objects use the updated defaults.

---

## 2. Technology

- Engine: Unreal Engine 5.6
- Language: C++
- UI: UMG with CommonUI
- Config: Unreal config system (GConfig, `[InteractiveObjectManager.Settings]` in Game.ini via GGameIni)
- Target platform: Windows desktop

---

## 3. High level architecture

The project is intentionally small and is split into three main parts.

- Game project `IOManager`  
  Standard UE 5.6 C++ project that owns maps and startup settings.

- Custom C++ module `InteractiveObjectManager`  
  Owns the logic for:
  - tracking spawned interactive objects
  - exposing functions for spawn, delete, selection and editing
  - providing Blueprint friendly APIs for the UI

- Settings and UI  
  - a small C++ settings class that reads and writes default values to a custom section in Game.ini using the Unreal config system  
  - a CommonUI based UMG screen with two tabs: Main (object list and controls) and Settings (ini backed defaults)

Most gameplay logic lives in C++. Blueprints are used mainly for UI binding and simple presentation.

---

## 4. Design choices

- Composition instead of a single base actor  
  Any actor can become interactive by adding a dedicated component instead of inheriting from a special base class.

- Clear API between C++ and UI  
  The UI calls a small set of Blueprint callable functions instead of touching game objects directly.

- Event driven updates  
  The UI is updated when objects or settings change, not on Tick.

- Config through a single settings wrapper  
  All ini access goes through one class that handles defaults, validation and logging.
  
---

## Making any Actor interactive

Any `AActor` can become interactive in this demo by adding the `UInteractiveObjectComponent`.

### What UInteractiveObjectComponent does

`UInteractiveObjectComponent` is responsible for visual control and registration of the Actor:

- Stores the current color (`FLinearColor CurrentColor`).
- Stores the current uniform scale (`float CurrentScale`).
- Locates the target `UStaticMeshComponent` on the owner Actor or uses an explicitly set `ScaleTargetComponent`.
- Creates dynamic material instances for all mesh materials on first color change.
- Applies color changes through the material parameter `ColorParameterName` (defaults to `BaseColor`).
- Applies uniform scale changes to the target component.
- Automatically registers and unregisters itself in `UInteractiveObjectManagerSubsystem` so that the Actor appears in the UI list and can be selected and modified.

The component does not use Tick. All changes are applied only when its methods are called explicitly.

Available public methods:

- `ApplyColor(const FLinearColor& NewColor)` - updates the stored color and applies it to the mesh.
- `ApplyScale(float NewScale)` - updates the stored scale and applies it to the Actor or the target component.
- `GetCurrentColor()` and `GetCurrentScale()` - return the current values that can be read by UI or other systems.

### Example usage in this demo

The demo project includes simple examples:

- `BP_InteractiveCube`
- `BP_InteractiveSphere`

Each blueprint:

- contains a `StaticMeshComponent` with the material that exposes a `BaseColor` parameter  
- has an `InteractiveObjectComponent` attached  
- is used by `UInteractiveObjectManagerSubsystem` when spawning new objects

When a new object is spawned, the subsystem looks up its `UInteractiveObjectComponent` and applies default color and scale from the settings.

To make any other Blueprint interactive:

1. Add a `StaticMeshComponent` with a material that has a vector color parameter (by default `BaseColor`).
2. Add the `InteractiveObjectComponent`.
3. Optionally adjust in Details:
   - `CurrentColor`
   - `CurrentScale`
   - `ColorParameterName`
   - `ScaleTargetComponent` if scale should be applied to a specific component
4. The component will register itself in the world subsystem on BeginPlay.
5. Call `ApplyColor` and `ApplyScale` from Blueprint or C++ or let the subsystem drive these values via defaults.

---

## Using the Interactive Object Manager

### Initial configuration

Before running the demo, configure the module in Project Settings:

1. Open **Edit → Project Settings → Game → Interactive Object Manager**.
2. In the **Primitive Classes** section:
   - Set **Cube Primitive Class** to your cube blueprint, for example `BP_InteractiveCube` under `/Game/InteractiveObjectManager/Actors`.
   - Set **Sphere Primitive Class** to your sphere blueprint, for example `BP_InteractiveSphere` under the same folder.
3. In the **Spawn Settings** section:
   - Adjust **Spawn Radius** to control how far from the world origin new objects can appear on the X/Y plane.
   - Adjust **Spawn Height** to control the Z offset for spawned objects.
4. Runtime defaults for spawn type, color and scale are stored in the ini section

   `[InteractiveObjectManager.Settings]`

   and are managed by `UInteractiveObjectSettings`.

   - **Default spawn type** drives which primitive is used by the Spawn button.
   - **Default color** and **default scale** are applied to a newly spawned object through `UInteractiveObjectComponent`.

### Running the demo

1. Open the provided demo level that contains the Interactive Object Manager UI overlay.
2. Press **Play in Editor**.

You should see the **Interactive Object Manager** panel at the top of the screen.

### Main tab

The Main tab is connected to `UInteractiveObjectManagerSubsystem` and allows you to work with all interactive objects in the current world.

- **Spawn object**  
  Calls `SpawnDefaultObject` on the subsystem.  
  - The actual primitive class (cube or sphere) is chosen based on the current default spawn type from `UInteractiveObjectSettings`.
  - Cube and sphere classes are resolved from the Project Settings described above.

- **Objects list**  
  - Shows all actors that have an attached `UInteractiveObjectComponent` and are registered in the subsystem.
  - Selecting a row changes the current selection in the subsystem. The selected item is highlighted in the list.
  - The list is displayed in a horizontal `ScrollBox`. Only a limited number of items are visible at once, the rest are accessible through horizontal scrolling.

- **Scale controls**  
  - Enter a numeric value in the **Scale** field and press **Apply scale**.
  - The subsystem calls `SetSelectedObjectUniformScale` on the currently selected object, which forwards the request to its `UInteractiveObjectComponent`.

- **Color controls**  
  - Enter values for **R**, **G**, and **B** and press **Apply color**.
  - The subsystem calls `SetSelectedObjectColor`, and the interactive component updates the dynamic material instance on the mesh.

- **Delete selected object**  
  - Removes the currently selected actor from the world, unregisters it from the subsystem and updates the UI list.
  - If there are other objects left, the first one in the list b

---