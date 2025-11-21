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
- Config: Unreal config system (GConfig, GameUserSettings.ini)
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
  - a small C++ settings class that reads and writes default values to a custom section in `GameUserSettings.ini` using the Unreal config system  
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

`UInteractiveObjectComponent` is responsible for local visual control of the Actor:

- Stores the current color (`FLinearColor CurrentColor`).
- Stores the current uniform scale (`float CurrentScale`).
- Locates the target `UStaticMeshComponent` on the owner Actor or uses an explicitly set `ScaleTargetComponent`.
- Creates dynamic material instances for all mesh materials on first color change.
- Applies color changes through the material parameter `ColorParameterName` (defaults to `BaseColor`).
- Applies uniform scale changes to the target component.

The component does not use Tick. All changes are applied only when its methods are called explicitly.

Available public methods:

- `ApplyColor(const FLinearColor& NewColor)` – updates the stored color and applies it to the mesh.
- `ApplyScale(float NewScale)` – updates the stored scale and applies it to the Actor or the target component.
- `GetCurrentColor()` and `GetCurrentScale()` – return the current values that can be read by UI or other systems.

### Example usage in this demo

The demo project includes a basic example:

- `BP_IOM_InteractiveObjectDemo`  
  - contains a `StaticMeshComponent` with the material `M_IOM_InteractiveObject` that exposes a `BaseColor` parameter  
  - has an `InteractiveObjectComponent` attached  
  - calls `ApplyColor` and `ApplyScale` in `BeginPlay` to demonstrate runtime color and scale changes

To make any other Blueprint interactive:

1. Add a `StaticMeshComponent` with a material that has a vector color parameter (by default `BaseColor`).
2. Add the `InteractiveObjectComponent`.
3. Optionally adjust in Details:
   - `CurrentColor`
   - `CurrentScale`
   - `ColorParameterName`
   - `ScaleTargetComponent` if scale should be applied to a specific component
4. Call `ApplyColor` and `ApplyScale` from Blueprint or C++ to change the visual appearance of the Actor at runtime.

---