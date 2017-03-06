/**
 * @file app/Camera.h
 * @author Tomas Polasek
 * @brief Camera implementation.
 */

#ifndef SIMPLE_GAME_CAMERA_H
#define SIMPLE_GAME_CAMERA_H

#include "Types.h"

/// OpenGL camera class.
class Camera
{
public:
    /**
     * Construct camera at origin, looking long the negative z axis.
     * @param fov Field of view in degrees.
     * @param aspectRatio Aspect ratio, usually width / height.
     * @param near Near clipping plane.
     * @param far Far clipping plane.
     */
    Camera(f32 fov, f32 aspectRatio, f32 near, f32 far)
    {
        calculatePerspectiveProjection(fov, aspectRatio, near, far);
    }

    /**
     * Move the camera to given position.
     * @param pos Position in the world-space.
     */
    void setPos(const glm::vec3 &pos)
    { calculateTranslation(pos); }

    /**
     * Rotate the camera using given vector of radian angles.
     * @param rot Vector containing angles in radians.
     */
    void setRot(const glm::vec3 &rot)
    { calculateRotation(rot); }

    /**
     * Get ViewProjection matrix.
     * @return Returns the ViewProjection matrix.
     */
    const glm::mat4 &viewProjectionMatrix()
    { return getVP(); }

    /**
     * Get address of the ViewProjection data.
     * @return Returns ptr to the first value in the ViewProjection matrix.
     */
    const glm::mat4::value_type *viewProjection()
    { return &getVP()[0][0]; }

    /**
     * Recalculate the ViewProjection matrix.
     * Must be manually called, if the changes to
     * other matrices should take effect.
     */
    void recalculate()
    {
        calculateView();
        calculateViewProjection();
    }
private:
    /**
     * Calculate Rotation matrix using quaternions. Input vector
     * has to be in radians.
     * @param rot Rotation (x, y, z) in radians.
     */
    void calculateRotation(const glm::vec3 &rot)
    {
        mRotationQuat = glm::quat(rot);
        mRotation = glm::mat4_cast(mRotationQuat);
    }

    /**
     * Calculate the Translation matrix using given position.
     * @param pos Position in world space.
     */
    void calculateTranslation(const glm::vec3 &pos)
    {
        mTranslation = glm::translate(glm::mat4(1.0f), -pos);
    }

    /// Calculate the View matrix.
    void calculateView()
    {
        mView = mRotation * mTranslation;
    }

    /**
     * Calculate the Projection matrix using perspective projection.
     * @param fov Field of view in degrees.
     * @param aspectRatio Aspect ratio, usually width / height.
     * @param near Near clipping plane.
     * @param far Far clipping plane.
     */
    void calculatePerspectiveProjection(f32 fov, f32 aspectRatio, f32 near, f32 far)
    {
        mProjection = glm::perspective(fov, aspectRatio, near, far);
    }

    /// Calculate the ViewProjection matrix.
    void calculateViewProjection()
    { mViewProjection = mProjection * mView; }

    /// Get the View matrix, if the dirty flag is set, it will be recalculated first.
    const glm::mat4 &getView()
    {
        return mView;
    }

    /// Get the Projection matrix.
    const glm::mat4 &getProjection()
    {
        return mProjection;
    }

    /**
     * Get the ViewProjection matrix. If the dirty flag
     * is set, it will be recalculated.
     * @return Returns the ViewProjection matrix.
     */
    const glm::mat4 &getVP()
    {
        return mViewProjection;
    }

    /// Translation matrix.
    glm::mat4 mTranslation;
    /// Rotation quaternion.
    glm::quat mRotationQuat;
    /// Rotation matrix.
    glm::mat4 mRotation;
    /// View matrix = mRotation * mTranslation.
    glm::mat4 mView;
    /// Projection matrix.
    glm::mat4 mProjection;
    /// ViewProjection matrix = mProjection * mView.
    glm::mat4 mViewProjection;
protected:
}; // class Camera

#endif //SIMPLE_GAME_CAMERA_H
