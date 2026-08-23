#!/usr/bin/env python3
from pathlib import Path
import vtk, sys

def render(gltf_path,out_path,az=35,el=20):
    rw=vtk.vtkRenderWindow(); rw.SetOffScreenRendering(1); rw.SetSize(1200,1000)
    ren=vtk.vtkRenderer(); ren.SetBackground(0.06,0.07,0.08); rw.AddRenderer(ren)
    imp=vtk.vtkGLTFImporter(); imp.SetFileName(str(gltf_path)); imp.SetRenderWindow(rw); imp.Update()
    # importer creates renderer(s); use first renderer if present
    rens=rw.GetRenderers(); rens.InitTraversal(); r=rens.GetNextItem()
    if r is None: r=ren
    # bounds from visible props
    r.ResetCamera(); cam=r.GetActiveCamera();
    # use azimuth/elevation on default fit
    cam.Azimuth(az); cam.Elevation(el); cam.OrthogonalizeViewUp(); r.ResetCameraClippingRange()
    rw.Render()
    w2i=vtk.vtkWindowToImageFilter(); w2i.SetInput(rw); w2i.SetScale(2); w2i.SetInputBufferTypeToRGBA(); w2i.ReadFrontBufferOff(); w2i.Update()
    wr=vtk.vtkPNGWriter(); wr.SetFileName(str(out_path)); wr.SetInputConnection(w2i.GetOutputPort()); wr.Write()

if __name__=='__main__': render(Path(sys.argv[1]),Path(sys.argv[2]), float(sys.argv[3]) if len(sys.argv)>3 else 35,float(sys.argv[4]) if len(sys.argv)>4 else 20)
