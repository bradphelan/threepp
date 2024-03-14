
#include "threepp/controls/FlyControls.hpp"

#include "threepp/canvas/Canvas.hpp"
#include "threepp/core/Object3D.hpp"
#include "threepp/math/Spherical.hpp"

using namespace threepp;

namespace {

    const float EPS = 0.000001f;

    struct MoveState {

        int up = 0;
        int down = 0;
        int left = 0;
        int right = 0;
        int forward = 0;
        int back = 0;
        float pitchUp = 0;
        float pitchDown = 0;
        float yawLeft = 0;
        float yawRight = 0;
        float rollLeft = 0;
        float rollRight = 0;
    };

}// namespace

struct FlyControls::Impl {

    Impl(FlyControls& scope, PeripheralsEventSource& canvas, Object3D* object)
        : eventSource(canvas), scope(scope), object(object) {

        subs_ << canvas.keys.Pressed.subscribe([this](auto& key) { onKeyPressed(key); });
        subs_ << canvas.keys.Released.subscribe([this](auto& key) { onKeyReleased(key); });

        subs_ << canvas.mouse.Down.subscribe([this](auto& e) { onMouseDown(e); });
        subs_ << canvas.mouse.Up.subscribe([this](auto& e) { onMouseUp(e); });
        subs_ << canvas.mouse.Move.subscribe([this](auto& e) { onMouseMove(e); });
    }

    void update(float delta) {

        const auto moveMult = delta * scope.movementSpeed;
        const auto rotMult = delta * scope.rollSpeed;

        object->translateX(moveVector.x * moveMult);
        object->translateY(moveVector.y * moveMult);
        object->translateZ(moveVector.z * moveMult);

        tmpQuaternion.set(rotationVector.x * rotMult, rotationVector.y * rotMult, rotationVector.z * rotMult, 1).normalize();
        object->quaternion.multiply(tmpQuaternion);

        if (
                lastPosition.distanceToSquared(object->position) > EPS ||
                8 * (1 - lastQuaternion.dot(object->quaternion)) > EPS) {

            lastQuaternion.copy(object->quaternion);
            lastPosition.copy(object->position);
        }
    }

    void updateMovementVector() {

        int forward = (moveState.forward || (scope.autoForward && !moveState.back));

        moveVector.x = static_cast<float>(-moveState.left + moveState.right);
        moveVector.y = static_cast<float>(-moveState.down + moveState.up);
        moveVector.z = static_cast<float>(-forward + moveState.back);
    }

    void updateRotationVector() {

        rotationVector.x = (-moveState.pitchDown + moveState.pitchUp);
        rotationVector.y = (-moveState.yawRight + moveState.yawLeft);
        rotationVector.z = (-moveState.rollRight + moveState.rollLeft);
    }

    void onKeyPressed(KeyEvent evt) {

        if (evt.mods == 4) {// altKey

            return;
        }

        switch (evt.key) {
            case Key::W:
                moveState.forward = 1;
                break;
            case Key::S:
                moveState.back = 1;
                break;
            case Key::A:
                moveState.left = 1;
                break;
            case Key::D:
                moveState.right = 1;
                break;
            case Key::R:
                moveState.up = 1;
                break;
            case Key::F:
                moveState.down = 1;
                break;
            case Key::UP:
                moveState.pitchUp = 1;
                break;
            case Key::DOWN:
                moveState.pitchDown = 1;
                break;
            case Key::LEFT:
                moveState.yawLeft = 1;
                break;
            case Key::RIGHT:
                moveState.yawRight = 1;
                break;
            case Key::Q:
                moveState.rollLeft = 1;
                break;
            case Key::E:
                moveState.rollRight = 1;
                break;
            default:
                break;
        }

        updateMovementVector();
        updateRotationVector();
    }

    void onKeyReleased(KeyEvent evt) {

        switch (evt.key) {
            case Key::W:
                moveState.forward = 0;
                break;
            case Key::S:
                moveState.back = 0;
                break;
            case Key::A:
                moveState.left = 0;
                break;
            case Key::D:
                moveState.right = 0;
                break;
            case Key::R:
                moveState.up = 0;
                break;
            case Key::F:
                moveState.down = 0;
                break;
            case Key::UP:
                moveState.pitchUp = 0;
                break;
            case Key::DOWN:
                moveState.pitchDown = 0;
                break;
            case Key::LEFT:
                moveState.yawLeft = 0;
                break;
            case Key::RIGHT:
                moveState.yawRight = 0;
                break;
            case Key::Q:
                moveState.rollLeft = 0;
                break;
            case Key::E:
                moveState.rollRight = 0;
                break;
            default:
                break;
        }

        updateMovementVector();
        updateRotationVector();
    }

    void onMouseDown(MouseButtonEvent& e) {

        if (scope.dragToLook) {

            mouseStatus++;

        } else {

            switch (e.button) {

                case 0:
                    moveState.forward = 1;
                    break;
                case 1:
                    moveState.back = 1;
                    break;
            }

            updateMovementVector();
        }
    }


    void onMouseMove(MouseEvent& e) {

        if (!scope.dragToLook || mouseStatus > 0) {

            const float halfWidth = static_cast<float>(eventSource.size().width) / 2;
            const float halfHeight = static_cast<float>(eventSource.size().height) / 2;

            moveState.yawLeft = -((e.pos.x) - halfWidth) / halfWidth;
            moveState.pitchDown = ((e.pos.y) - halfHeight) / halfHeight;

            updateRotationVector();
        }
    }


    void onMouseUp(MouseButtonEvent& e) {

        if (scope.dragToLook) {

            mouseStatus--;

            moveState.yawLeft = moveState.pitchDown = 0;

        } else {

            switch (e.button) {

                case 0:
                    moveState.forward = 0;
                    break;
                case 1:
                    moveState.back = 0;
                    break;
            }

            updateMovementVector();
        }

        updateRotationVector();
    }

private:
    PeripheralsEventSource& eventSource;
    FlyControls& scope;
    Object3D* object;

    Quaternion lastQuaternion;
    Vector3 lastPosition;

    Quaternion tmpQuaternion;

    int mouseStatus = 0;

    MoveState moveState;
    Vector3 moveVector;
    Vector3 rotationVector;

    std::vector<Subscription> subs_;
};

FlyControls::FlyControls(Object3D& object, PeripheralsEventSource& eventSource)
    : pimpl_(std::make_unique<Impl>(*this, eventSource, &object)) {}

void FlyControls::update(float delta) {

    pimpl_->update(delta);
}

FlyControls::~FlyControls() = default;
