// SPDX-FileCopyrightText: Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
// SPDX-License-Identifier: BSD-3-Clause

/*
 * This work (vtkInteractorStyleUnicam.h) was produced under a grant from
 * the Department of Energy to Brown University.
 */

/**
 * @class   vtkInteractorStyleUnicam
 * @brief   provides Unicam navigation style
 *
 * UniCam is a camera interactor.  Here, just the primary features of the
 * UniCam technique are implemented.  UniCam requires just one mouse button
 * and supports context sensitive dollying, panning, and rotation.  (In this
 * implementation, it uses the right mouse button, leaving the middle and
 * left available for other functions.) For more information, see the paper
 * at:
 *
 *    ftp://ftp.cs.brown.edu/pub/papers/graphics/research/unicam.pdf
 *
 * The following is a brief description of the UniCam Camera Controls.  You
 * can perform 3 operations on the camera: rotate, pan, and dolly the camera.
 * All operations are reached through the right mouse button & mouse
 * movements.
 *
 * IMPORTANT: UniCam assumes there is an axis that makes sense as a "up"
 * vector for the world.  By default, this axis is defined to be the
 * vector <0,0,1>.  You can set it explicitly for the data you are
 * viewing with the 'SetWorldUpVector(..)' method.
 *
 * 1. ROTATE:
 *
 * Position the cursor over the point you wish to rotate around and press and
 * release the left mouse button.  A 'focus dot' appears indicating the
 * point that will be the center of rotation.  To rotate, press and hold the
 * left mouse button and drag the mouse.. release the button to complete the
 * rotation.
 *
 * Rotations can be done without placing a focus dot first by moving the
 * mouse cursor to within 10% of the window border & pressing and holding the
 * left button followed by dragging the mouse.  The last focus dot position
 * will be reused.
 *
 * 2. PAN:
 *
 * Click and hold the left mouse button, and initially move the mouse left
 * or right.  The point under the initial pick will pick correlate w/ the
 * mouse tip-- (i.e., direct manipulation).
 *
 * 3. DOLLY (+ PAN):
 *
 * Click and hold the left mouse button, and initially move the mouse up or
 * down.  Moving the mouse down will dolly towards the picked point, and moving
 * the mouse up will dolly away from it.  Dollying occurs relative to the
 * picked point which simplifies the task of dollying towards a region of
 * interest. Left and right mouse movements will pan the camera left and right.
 *
 * @warning
 * (NOTE: This implementation of Unicam assumes a perspective camera.  It
 * could be modified relatively easily to also support an orthographic
 * projection.)
 */

#ifndef vtkInteractorStyleUnicam_h
#define vtkInteractorStyleUnicam_h

#include "vtkInteractionStyleModule.h" // For export macro
#include "vtkInteractorStyle.h"
#include "vtkWrappingHints.h" // For VTK_MARSHALAUTO

VTK_ABI_NAMESPACE_BEGIN
class vtkCamera;
class vtkWorldPointPicker;

class VTKINTERACTIONSTYLE_EXPORT VTK_MARSHALAUTO vtkInteractorStyleUnicam
  : public vtkInteractorStyle
{
public:
  enum
  {
    NONE = 0,
    BUTTON_LEFT = 1,
    BUTTON_MIDDLE = 2,
    BUTTON_RIGHT = 3
  };
  enum
  {
    CAM_INT_ROT = 0,
    CAM_INT_CHOOSE = 1,
    CAM_INT_PAN = 2,
    CAM_INT_DOLLY = 3
  };

  static vtkInteractorStyleUnicam* New();
  vtkTypeMacro(vtkInteractorStyleUnicam, vtkInteractorStyle);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  void SetWorldUpVector(double a[3]) { this->SetWorldUpVector(a[0], a[1], a[2]); }
  void SetWorldUpVector(double x, double y, double z);
  vtkGetVectorMacro(WorldUpVector, double, 3);

  ///@{
  /**
   * Concrete implementation of event bindings
   */
  void OnMouseMove() override;
  void OnLeftButtonDown() override;
  void OnLeftButtonUp() override;
  virtual void OnLeftButtonMove();
  ///@}

  /**
   * OnTimer calls RotateCamera, RotateActor etc which should be overridden by
   * style subclasses.
   */
  void OnTimer() override;

protected:
  vtkInteractorStyleUnicam();
  ~vtkInteractorStyleUnicam() override;

  vtkWorldPointPicker* InteractionPicker;

  int ButtonDown;     // which button is down
  double DTime;       // time mouse button was pressed
  double Dist;        // distance the mouse has moved since button press
  double StartPix[2]; // pixel mouse movement started at
  double LastPos[2];  // normalized position of mouse last frame
  double LastPix[2];  // pixel position of mouse last frame
  double DownPt[3];   // 3D point under cursor when mouse button pressed
  double Center[3];   // center of camera rotation

  double WorldUpVector[3]; // what the world thinks the 'up' vector is

  vtkActor* FocusSphere;            // geometry for indicating center of rotation
  int IsDot;                        // flag-- is the FocusSphere being displayed?
  vtkRenderer* FocusSphereRenderer; // renderer for 'FocusSphere'

  int state; // which navigation mode was selected?

  void ChooseXY(int X, int Y); // method for choosing type of navigation
  void RotateXY(int X, int Y); // method for rotating
  void DollyXY(int X, int Y);  // method for dollying
  void PanXY(int X, int Y);    // method for panning

  // convenience methods for translating & rotating the camera
  void MyTranslateCamera(double v[3]);
  void MyRotateCamera(
    double cx, double cy, double cz, double ax, double ay, double az, double angle);

  // Given a 3D point & a vtkCamera, compute the vectors that extend
  // from the projection of the center of projection to the center of
  // the right-edge and the center of the top-edge onto the plane
  // containing the 3D point & with normal parallel to the camera's
  // projection plane.
  void GetRightVandUpV(double* p, vtkCamera* cam, double* rightV, double* upV);

  // takes in pixels, returns normalized window coordinates
  void NormalizeMouseXY(int X, int Y, double* NX, double* NY);

  // return the aspect ratio of the current window
  double WindowAspect();

private:
  vtkInteractorStyleUnicam(const vtkInteractorStyleUnicam&) = delete;
  void operator=(const vtkInteractorStyleUnicam&) = delete;
};

VTK_ABI_NAMESPACE_END
#endif // vtkInteractorStyleUnicam_h
