### **MiniRT Scene File Format (`.rt`) Documentation**

This document describes the syntax for creating scene files for the miniRT renderer.

#### **General Syntax**

*   Each line defines a single element in the scene.
*   Each line starts with a type identifier (e.g., `A`, `C`, `sp`), followed by space-separated parameters.
*   Lines starting with a `#` are considered comments and are ignored.
*   Empty lines or lines with only whitespace are ignored.
*   Values can be integers or floating-point numbers.

#### **Core Data Types**

These are the basic building blocks used by multiple elements.

*   **`Vec3`**: A 3D vector used for positions, orientations, and normals.
    *   **Format**: `x,y,z` (e.g., `10.5,0.0,-5.0`)
    *   **Note**: Orientation/axis vectors are automatically normalized by the parser.

*   **`Color`**: An RGB color value.
    *   **Format**: `R,G,B` (e.g., `255,127,0`)
    *   **Note**: The parser is smart. You can provide values in the **0-255** range, and they will be automatically normalized to the **0.0-1.0** range required by the renderer. You can also provide values directly in the 0.0-1.0 range.

---

### **Scene Elements**

A valid scene file **must contain exactly one** Ambient Light (`A`) and one Camera (`C`).

#### **`A` - Ambient Light (Mandatory)**

Defines the global ambient light for the entire scene.

*   **Format**: `A ratio color`
*   **Parameters**:
    *   `ratio`: A single value for the light's intensity (e.g., `0.2`).
    *   `color`: The `Color` of the ambient light.
*   **Example**:
    ```
    A 0.1 255,255,255  # A faint, white ambient light
    ```

#### **`C` - Camera (Mandatory)**

Defines the viewpoint from which the scene is rendered.

*   **Format**: `C position orientation fov`
*   **Parameters**:
    *   `position`: A `Vec3` defining the camera's location.
    *   `orientation`: A `Vec3` defining the direction the camera is looking.
    *   `fov`: A single value for the horizontal Field of View in degrees (must be between 0 and 180).
*   **Example**:
    ```
    C 0,0,0 0,0,1 70   # Camera at the origin, looking along the Z-axis, with a 70-degree FOV
    ```

#### **`L` - Light (Optional, Multiple Allowed)**

Defines a point light source in the scene. You can add multiple lights.

*   **Format**: `L position brightness color`
*   **Parameters**:
    *   `position`: A `Vec3` defining the light's location.
    *   `brightness`: A single value for the light's intensity (e.g., `0.8`).
    *   `color`: The `Color` of the light.
*   **Example**:
    ```
    L -10,10,15 0.7 255,255,255 # A bright white light
    ```

---

### **Geometric Objects**

These are the visible shapes in your scene. They all share a common set of material properties.

#### **Material Properties**

Material properties are defined after the geometric parameters for each object.

*   **`diffuse_color` (Required)**: The base `Color` of the object. This is always the first color parameter.
*   **Optional Properties**: If you don't provide these, they will use default values.
    *   `specular_color`: The color of the shiny highlight. (Default: `0.5,0.5,0.5` - medium gray)
    *   `shininess`: A value controlling the size and sharpness of the highlight. Higher values mean smaller, sharper highlights. (Default: `32.0`)
    *   `reflectivity`: A value from 0.0 to 1.0 controlling how much the object reflects its surroundings. (Default: `0.0`)

**Supported Formats:**

The parser is flexible. Here are the ways you can define materials, from simplest to most complex:

1.  **Diffuse Color Only**
    *   `... 255,0,127`

2.  **Diffuse + Combined Specular/Shininess**
    *   `... 255,0,127 "0.8,128"` (Gray specular highlight with 0.8 intensity and 128 shininess)
    *   `... 255,0,127 "255,255,0,256"` (Yellow specular highlight with 256 shininess)

3.  **Diffuse + Combined Specular/Shininess + Reflectivity**
    *   `... 255,0,127 "0.8,128" 0.5` (Adds 0.5 reflectivity)

4.  **Diffuse + Separate Specular Color + Shininess + Reflectivity**
    *   `... 255,0,127 255,255,255 128 0.5` (White specular, 128 shininess, 0.5 reflectivity)

#### **`sp` - Sphere**

*   **Format**: `sp center diameter diffuse_color [material_properties...]`
*   **Parameters**:
    *   `center`: `Vec3` position of the sphere's center.
    *   `diameter`: A single value for the sphere's diameter.
*   **Examples**:
    ```
    # A simple red sphere
    sp 0,0,30 10 255,0,0

    # A shiny, reflective blue sphere
    sp 15,5,40 8 0,0,255 "1.0,256" 0.75
    ```

#### **`pl` - Plane**

*   **Format**: `pl point normal diffuse_color [material_properties...]`
*   **Parameters**:
    *   `point`: A `Vec3` position of any point on the infinite plane.
    *   `normal`: A `Vec3` defining the plane's normal vector (which way it's "facing").
*   **Example**:
    ```
    # A gray floor plane
    pl 0,-10,0 0,1,0 128,128,128
    ```

#### **`cy` - Cylinder**

*   **Format**: `cy center axis diameter height diffuse_color [material_properties...]`
*   **Parameters**:
    *   `center`: `Vec3` position of the center of the cylinder.
    *   `axis`: `Vec3` defining the orientation of the cylinder's central axis.
    *   `diameter`: A single value for the cylinder's diameter.
    *   `height`: A single value for the cylinder's height.
*   **Example**:
    ```
    # A green cylinder standing upright
    cy 0,0,25 0,1,0 10 20 0,255,0 "0.5,64"
    ```

---

### **Complete Example File**

```rt
# ----------------------------------
# Example Scene for miniRT
# ----------------------------------

# Mandatory Elements: Ambient Light and Camera
A 0.1 255,255,255
C -50,20,0 1,0,0 75

# A bright, white main light source
L 0,50,0 0.9 255,255,255

# --- Objects ---

# A large, matte red sphere in the center
sp 0,0,0 20 255,0,0

# A small, highly reflective and shiny blue sphere on the side
sp 25,5,-10 10 0,127,255 "1.0,512" 0.8

# A floor plane with a slight gray specular highlight
pl 0,-10,0 0,1,0 150,150,150 "0.2,32"

# A green cylinder
cy -20,0,20 0.707,0.707,0 8 15 0,255,127 "255,255,255,128"
```