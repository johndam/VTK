// SPDX-FileCopyrightText: Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
// SPDX-License-Identifier: BSD-3-Clause

// Description
// This tests the circle resolution parameter for vtkGlyphSource2D

#include "vtkActor2D.h"
#include "vtkFloatArray.h"
#include "vtkGlyph2D.h"
#include "vtkGlyphSource2D.h"
#include "vtkMinimalStandardRandomSequence.h"
#include "vtkNew.h"
#include "vtkPointData.h"
#include "vtkPoints.h"
#include "vtkPolyData.h"
#include "vtkPolyDataMapper2D.h"
#include "vtkRegressionTestImage.h"
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkRenderer.h"
#include "vtkTestUtilities.h"

int TestGlyphSource2DResolution(int argc, char* argv[])
{
  cout << "CTEST_FULL_OUTPUT (Avoid ctest truncation of output)" << endl;

  vtkNew<vtkPolyData> pd;
  vtkNew<vtkPoints> pts;

  vtkNew<vtkFloatArray> scalars;
  vtkNew<vtkFloatArray> vectors;
  vectors->SetNumberOfComponents(3);

  pd->SetPoints(pts);
  pd->GetPointData()->SetScalars(scalars);
  pd->GetPointData()->SetVectors(vectors);

  vtkNew<vtkMinimalStandardRandomSequence> randomSequence;
  randomSequence->SetSeed(1);

  int size = 400;

  for (int i = 0; i < 100; ++i)
  {
    randomSequence->Next();
    double x = randomSequence->GetValue() * size;
    randomSequence->Next();
    double y = randomSequence->GetValue() * size;
    pts->InsertNextPoint(x, y, 0.0);
    randomSequence->Next();
    scalars->InsertNextValue(5.0 * randomSequence->GetValue());
    randomSequence->Next();
    double ihat = randomSequence->GetValue() * 2 - 1;
    randomSequence->Next();
    double jhat = randomSequence->GetValue() * 2 - 1;
    vectors->InsertNextTuple3(ihat, jhat, 0.0);
  }

  vtkNew<vtkGlyphSource2D> gs;
  gs->SetGlyphTypeToCircle();
  gs->SetScale(20);
  gs->FilledOff();
  gs->CrossOn();

  vtkNew<vtkGlyphSource2D> gs1;
  gs1->SetGlyphTypeToCircle();
  gs1->SetResolution(24);
  gs1->SetScale(30);
  gs1->FilledOn();
  gs1->CrossOff();

  vtkNew<vtkGlyphSource2D> gs2;
  gs2->SetGlyphTypeToCircle();
  gs2->SetResolution(6);
  gs2->SetScale(20);
  gs2->FilledOn();
  gs2->CrossOff();

  vtkNew<vtkGlyphSource2D> gs3;
  gs3->SetGlyphTypeToCircle();
  gs3->SetResolution(5);
  gs3->SetScale(30);
  gs3->FilledOff();
  gs3->CrossOn();

  vtkNew<vtkGlyphSource2D> gs4;
  gs4->SetGlyphTypeToCircle();
  gs4->SetResolution(100);
  gs4->SetScale(50);
  gs4->FilledOff();
  gs4->CrossOff();

  vtkNew<vtkGlyph2D> glypher;
  glypher->SetInputData(pd);
  glypher->SetSourceConnection(0, gs->GetOutputPort());
  glypher->SetSourceConnection(1, gs1->GetOutputPort());
  glypher->SetSourceConnection(2, gs2->GetOutputPort());
  glypher->SetSourceConnection(3, gs3->GetOutputPort());
  glypher->SetSourceConnection(4, gs4->GetOutputPort());
  glypher->SetIndexModeToScalar();
  glypher->SetRange(0, 5);
  glypher->SetScaleModeToScaleByVector();

  vtkNew<vtkPolyDataMapper2D> mapper;
  mapper->SetInputConnection(glypher->GetOutputPort());
  mapper->SetScalarRange(0, 5);

  vtkNew<vtkActor2D> glyphActor;
  glyphActor->SetMapper(mapper);

  // Create the RenderWindow, Renderer
  vtkNew<vtkRenderWindow> renWin;
  renWin->SetMultiSamples(0);
  vtkNew<vtkRenderWindowInteractor> iren;
  iren->SetRenderWindow(renWin);

  vtkNew<vtkRenderer> ren;
  ren->AddViewProp(glyphActor);
  ren->SetBackground(0.3, 0.3, 0.3);
  ren->ResetCamera();

  renWin->SetSize(size + 1, size - 1); // NPOT size
  renWin->AddRenderer(ren);
  renWin->Render();

  iren->Initialize();

  int retVal = vtkRegressionTestImage(renWin);
  if (retVal == vtkRegressionTester::DO_INTERACTOR)
  {
    iren->Start();
  }

  return !retVal;
}
