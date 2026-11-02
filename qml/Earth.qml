// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick
import QtQuick3D

Rectangle {
    id: root
    objectName: "earthScene"
    color: "#020304"
    property real yaw: 0
    property real pitch: -12
    property real cameraDistance: 315
    readonly property real minimumDistance: 165
    readonly property real maximumDistance: 650

    function resetCamera() {
        yaw = 0
        pitch = -12
        cameraDistance = 315
    }
    function zoomBy(delta) {
        cameraDistance = Math.max(minimumDistance, Math.min(maximumDistance,
                                    cameraDistance * Math.exp(-delta * 0.001)))
    }

    // A dark exposure keeps the focus on the sunlit Earth rather than decorative stars.
    Rectangle { anchors.fill: parent; color: "#020304" }

    View3D {
        id: space
        objectName: "earthViewport"
        anchors.fill: parent
        environment: SceneEnvironment {
            backgroundMode: SceneEnvironment.Transparent
            antialiasingMode: SceneEnvironment.MSAA
            antialiasingQuality: SceneEnvironment.High
            tonemapMode: SceneEnvironment.TonemapModeFilmic
        }
        camera: camera
        Node {
            eulerRotation: Qt.vector3d(root.pitch, root.yaw, 0)
            PerspectiveCamera {
                id: camera
                objectName: "earthCamera"
                z: root.cameraDistance
                clipNear: 1
                clipFar: 2000
                fieldOfView: 42
            }
        }
        DirectionalLight {
            eulerRotation: Qt.vector3d(-18, -70, 0)
            brightness: 1.35
            color: "#ffffff"
            ambientColor: "#243447"
        }
        Model {
            id: earth
            objectName: "earthGlobe"
            source: "#Sphere"
            scale: Qt.vector3d(2, 2, 2)
            eulerRotation: Qt.vector3d(0, -5, 15)
            materials: CustomMaterial {
                property vector3d sunDirection: Qt.vector3d(-0.89, 0.31, 0.32)
                property TextureInput dayMap: TextureInput {
                    texture: Texture { source: "../assets/earth-clouds.jpg"; generateMipmaps: true; mipFilter: Texture.Linear }
                }
                property TextureInput nightMap: TextureInput {
                    texture: Texture { source: "../assets/earth-night.jpg"; generateMipmaps: true; mipFilter: Texture.Linear }
                }
                fragmentShader: "earth-surface.frag"
            }
        }
        Model {
            source: "#Sphere"
            scale: Qt.vector3d(2.012, 2.012, 2.012)
            materials: CustomMaterial {
                property vector3d sunDirection: Qt.vector3d(-0.89, 0.31, 0.32)
                shadingMode: CustomMaterial.Unshaded
                cullMode: Material.BackFaceCulling
                sourceBlend: CustomMaterial.SrcAlpha
                destinationBlend: CustomMaterial.OneMinusSrcAlpha
                vertexShader: "atmosphere.vert"
                fragmentShader: "atmosphere.frag"
            }
        }
    }
    MouseArea {
        anchors.fill: parent
        acceptedButtons: Qt.LeftButton
        property point lastPosition
        cursorShape: pressed ? Qt.ClosedHandCursor : Qt.OpenHandCursor
        onPressed: mouse => { lastPosition = Qt.point(mouse.x, mouse.y) }
        onPositionChanged: mouse => {
            if (!pressed) return
            root.yaw -= (mouse.x - lastPosition.x) * 0.35
            root.pitch = Math.max(-85, Math.min(85, root.pitch - (mouse.y - lastPosition.y) * 0.35))
            lastPosition = Qt.point(mouse.x, mouse.y)
        }
        onWheel: wheel => { root.zoomBy(wheel.angleDelta.y); wheel.accepted = true }
        onDoubleClicked: root.resetCamera()
    }
    Column {
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.margins: 24
        spacing: 7
        Text { text: "EARTH"; color: "#e0e4e8"; font.pixelSize: 19; font.weight: Font.Medium }
        Text { text: "REFERENCE GLOBE"; color: "#737e89"; font.pixelSize: 10; font.letterSpacing: 0.5 }
    }
    Rectangle {
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.margins: 20
        width: resetLabel.width + 26
        height: 32
        radius: 6
        color: resetMouse.containsMouse ? "#2c323a" : "#191e25"
        border.color: "#414952"
        Text { id: resetLabel; anchors.centerIn: parent; text: "Reset view"; color: "#c3cbd3"; font.pixelSize: 12 }
        MouseArea { id: resetMouse; anchors.fill: parent; hoverEnabled: true; cursorShape: Qt.PointingHandCursor; onClicked: root.resetCamera() }
    }
    Text {
        anchors.left: parent.left
        anchors.bottom: parent.bottom
        anchors.margins: 20
        text: "Drag to rotate   ·   Scroll to zoom   ·   Double-click to reset"
        color: "#8a949e"
        font.pixelSize: 12
    }
    Text {
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.margins: 20
        text: "Imagery: NASA / NOAA"
        color: "#66717b"
        font.pixelSize: 10
        visible: root.width > 750
    }
}
