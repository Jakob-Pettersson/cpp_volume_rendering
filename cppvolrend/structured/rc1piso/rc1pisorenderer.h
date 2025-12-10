/**
 * Isosurface raycaster.
 *
 * @author Tino Weinkauf
**/
#pragma once

#include "../../volrenderbase.h"

class RayCasting1PassIso : public BaseVolumeRenderer
{
public:
  RayCasting1PassIso();
  virtual ~RayCasting1PassIso();
  
  virtual const char* GetName();
  virtual const char* GetAbbreviationName();
  virtual vis::GRID_VOLUME_DATA_TYPE GetDataTypeSupport();

  virtual void Clean();
  virtual bool Init(int shader_width, int shader_height);
  virtual void ReloadShaders();

  virtual bool Update(vis::Camera* camera);
  virtual void Redraw();

  virtual void SetImGuiComponents();

protected:
  float m_u_isovalue;
  bool m_apply_gradient_shading;

  float m_u_stepsize;
  glm::vec4 m_u_surfacecolor;

private:
  gl::ComputeShader* cp_shader_rendering;

};

