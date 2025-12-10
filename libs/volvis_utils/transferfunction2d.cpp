#include "TransferFunction2D.h"
#include <gl_utils/texture1d.h>
#include <gl_utils/texture2d.h>
#include <GL/glew.h>

#include <fstream>
#include <cstdlib>

#include <im/im.h>
#include <im/im_image.h>
#include <im/im_util.h>
#include "BMP.h"
namespace vis
{
  TransferFunction2D::TransferFunction2D (int max_value)
    : m_built (false)
  {
    max_density = max_value;

    m_cpt_rgb.clear ();
    m_cpt_alpha.clear ();
    m_transferfunction = NULL;

    extinction_coef_type = false;
    image_path = "";
  }

  TransferFunction2D::~TransferFunction2D ()
  {
    m_cpt_rgb.clear ();
    m_cpt_alpha.clear ();
    if (m_transferfunction)
      m_transferfunction;
    if (m_gradients)
      m_gradients;
  }

  const char* TransferFunction2D::GetNameClass ()
  {
    return "TrasnferFunction2D";
  }

  void TransferFunction2D::SetExtinctionCoefficientInput(bool s)
  {
    extinction_coef_type = s;
  }

  void TransferFunction2D::SetImagePath(std::string path)
  {
    image_path = path;
  }

  void TransferFunction2D::AddRGBControlPoint (TransferControlPoint rgb)
  {
    m_cpt_rgb.push_back (rgb);
  }

  void TransferFunction2D::AddAlphaControlPoint (TransferControlPoint alpha)
  {
    m_cpt_alpha.push_back (alpha);
  }

  void TransferFunction2D::ClearControlPoints ()
  {
    m_cpt_rgb.clear ();
    m_cpt_alpha.clear ();
  }

  gl::Texture2D* TransferFunction2D::GenerateTexture_2D_RGBt()
  {
    BMP bmp(image_path.c_str());
    int tf_size_x = max_density + 1;
    int tf_size_y = max_density + 1;
    gl::Texture2D* ret = new gl::Texture2D(tf_size_x, tf_size_y);
    ret->GenerateTexture(GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
    float* data = new float[tf_size_x * tf_size_y * 4];
    for (int x = 0; x < tf_size_x; x++) {
        for (int y = 0; y < tf_size_y; y++)
        {
            glm::vec4 colors = bmp.get_pixel(x, y);
            data[((x + tf_size_x * y) * 4)] = (float)colors[0]/255.f;
            data[((x + tf_size_x * y) * 4) + 1] = (float)colors[1]/255.f;
            data[((x + tf_size_x * y) * 4) + 2] = (float)colors[2]/255.f;
            data[((x + tf_size_x * y) * 4) + 3] = (float)colors[3]/255.f;
        }
    }
    ret->SetData((void*)data, GL_RGBA16F, GL_RGBA, GL_FLOAT);
    delete[] data;
    return ret;


    /*if (!m_built)
      Build();

    if (m_transferfunction)
    {
      int tf_size = max_density + 1;
        int error;
        imFile* tfImage = imFileOpen(image_path.c_str(), &error);
        if (error) {
            return NULL;
        }
        float* data = new float[tf_size * tf_size * 4];
		error = imFileReadImageData(tfImage, data, -1, IM_ALPHA);
        if (error) {
			printf("Error reading transfer function image data\n");
            return NULL;
        }
		gl::Texture2D* ret = new gl::Texture2D(tf_size, tf_size);
        std::string path_to_data = image_path;
      ret->GenerateTexture(GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
      ret->SetData((void*)data, GL_RGBA16F, GL_RGBA, GL_FLOAT);
      return ret;
*/

      /*gl::Texture1D* ret = new gl::Texture1D(tf_size);
      ret->GenerateTexture(GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
      float* data = new float[tf_size * 4];
      for (int i = 0; i < tf_size; i++)
      {
        data[(i * 4)]     = (float)(m_transferfunction[i].r);
        data[(i * 4) + 1] = (float)(m_transferfunction[i].g);
        data[(i * 4) + 2] = (float)(m_transferfunction[i].b);

        float v4 = (float)(m_transferfunction[i].a);

        if (!extinction_coef_type)
          v4 = MaterialOpacityToExtinction(v4);
        
        data[(i * 4) + 3] = v4;
      }
      ret->SetData((void*)data, GL_RGBA16F, GL_RGBA, GL_FLOAT);
      delete[] data;
      return ret;*/
  }

  void TransferFunction2D::Build ()
  {
    if (m_transferfunction)
      delete[] m_transferfunction;
    m_transferfunction = new glm::dvec4[max_density + 1];

    BuildLinear();

    printf ("lqc: Transfer Function 1D Built!\n");
    m_built = true;
  }

  glm::vec4 TransferFunction2D::Get (double valueX, double valueY, double max_data_value)
  {
	  BMP bmp(image_path.c_str());
      printf("BMP created\n");
	  glm::vec4 color = bmp.get_pixel(0, 50);
	  printf("R: %d, G: %d, B: %d, A: %d\n", (int)color[0], (int)color[1], (int)color[2], (int)color[3]);
     /* void* buffer = NULL;
      imFile* tfImage = NULL;

      int error;
	  tfImage = imFileOpen(image_path.c_str(), &error);
      if (error) {
			printf("Error opening transfer function image file\n");
            return glm::vec4(0);
      }
      int size = 0;
	  int width, height, color_mode, data_type;
      error = imFileReadImageInfo(tfImage, 0, &width, &height, &color_mode, &data_type);
      if (error) {
		  printf("Error reading image info: %d\n", error);
      }
	  printf("Width: %d, Height: %d, Color Mode: %d, Data Type: %d\n", width, height, color_mode, data_type);
      size = imImageDataSize(width, height, color_mode, data_type);
	  printf("Size: %d\n", size);
	  buffer = malloc(size);
	  error = imFileReadImageData(tfImage, buffer, 0, -1);
      if (error) {
			printf("Error reading image data: %d\n", error);
            return glm::vec4(0);
      }
	  unsigned char* data = (unsigned char*)buffer;
      printf("Data 0: %c\n", data[0]);*/

    /*int tf_size = max_density + 1;
        int error;
		printf("Opening transfer function image: %s\n", image_path.c_str());
        imFile* tfImage = imFileOpen(image_path.c_str(), &error);
        if (error) {
			printf("Error opening transfer function image file\n");
            return glm::vec4(0);
        }
		int width, height, color_mode, data_type;
        error = imFileReadImageInfo(tfImage, 0, &width, &height, &color_mode, &data_type);
        float* data = new float[width * height * 4];
		error = imFileReadImageData(tfImage, (void*)data, 0, -1);
        if (error) {
			printf("Error reading image data: %d\n", error);
            return glm::vec4(0);
        }
        printf("Data: %.4f, %.4f, %.4f\n", &data[0], &data[1], &data[2]);*/

        return glm::vec4(0);

    //if (!m_built)
    //  Build();

    //if (max_data_value >= 0)
    //  valueX = valueX * (double(max_density) / max_data_value);
    //  valueY = valueY * (double(max_density) / max_data_value);

    //if (valueX < 0.0f || valueX > (float)max_density || valueY < 0.0f || valueY > (float)max_density)
    //  return glm::vec4 (0);

    //// range: [0, max_density]
    //if (fabs(valueX - (float)max_density) < 0.000001 && fabs(valueY - (float)max_density) < 0.000001)
    //{
    //  return glm::vec4(m_transferfunction[max_density]);
    //}
    //else if (fabs(valueX - (float)max_density) < 0.000001)
    //{
    //  return glm::vec4(m_transferfunction[max_density]);
    //}
    //else
    //{
    //  glm::dvec4 v1 = m_transferfunction[(int)value];
    //  glm::dvec4 v2 = m_transferfunction[((int)value) + 1];

    //  double t = value - (int)value;

    //  return glm::vec4((1.0 - t)*v1 + t*v2);
    //}
  }

  float TransferFunction2D::GetOpc (double value, double max_input_value)
  {
    float val = Get(value, max_input_value).a;

    if (extinction_coef_type)
      return ExtinctionToMaterialOpacity(val);

    return val;
  }

  float TransferFunction2D::GetOpcN (double normalized_value)
  {
    float val = Get(normalized_value, 1.0).a;

    if (extinction_coef_type)
      return ExtinctionToMaterialOpacity(val);

    return val;
  }

  float TransferFunction2D::GetExt (double value, double max_input_value)
  {
    float val = Get(value, max_input_value).a;

    if (!extinction_coef_type)
      return MaterialOpacityToExtinction(val);

    return val;
  }

  float TransferFunction2D::GetExtN (double normalized_value)
  {
    float val = Get(normalized_value, 1.0).a;

    if (!extinction_coef_type)
      return MaterialOpacityToExtinction(val);

    return val;
  }

  void TransferFunction2D::PrintControlPoints ()
  {
    printf ("Print Transfer Function: Control Points\n");
    int rgb_pts = (int)m_cpt_rgb.size ();
    printf ("- Printing the RGB Control Points\n");
    printf ("  Format: \"Number: Red Green Blue, Isovalue\"\n");
    for (int i = 0; i < rgb_pts; i++)
    {
      printf ("  %d: %.2f %.2f %.2f, %d\n", i + 1, m_cpt_rgb[i].m_color.x, m_cpt_rgb[i].m_color.y, m_cpt_rgb[i].m_color.z, m_cpt_rgb[i].m_isoValue);
    }
    printf ("\n");

    int alpha_pts = (int)m_cpt_alpha.size ();
    printf ("- Printing the Alpha Control Points\n");
    printf ("  Format: \"Number: Alpha, Isovalue\"\n");
    for (int i = 0; i < alpha_pts; i++)
    {
      printf ("  %d: %.2f, %d\n", i + 1, m_cpt_alpha[i].m_color.w, m_cpt_alpha[i].m_isoValue);
    }
    printf ("\n");
  }

  void TransferFunction2D::PrintTransferFunction ()
  {
    printf ("Print Transfer Function: Control Points\n");
    printf ("  Format: \"IsoValue: Red Green Blue, Alpha\"\n");
    for (int i = 0; i < max_density + 1; i++)
    {
      printf ("%d: %.2f %.2f %.2f, %.2f\n", i, m_transferfunction[i].x
        , m_transferfunction[i].y, m_transferfunction[i].z, m_transferfunction[i].w);
    }
  }

  bool TransferFunction2D::Save ()//char* filename, TFFormatType format)
  {
    /*
    std::string filesaved;
    filesaved.append (RESOURCE_LIBLQC_PATH);
    filesaved.append ("TransferFunctions/");
    filesaved.append (filename);
    if (format == TFFormatType::LQC)
    {
    filesaved.append (".tf1d");
    std::ofstream myfile (filesaved.c_str ());
    if (myfile.is_open ())
    {
    myfile << 0 << "\n";
    myfile << (int)m_cpt_rgb.size () << "\n";
    for (int i = 0; i < (int)m_cpt_rgb.size (); i++)
    {
    myfile << m_cpt_rgb[i].m_color.x << " " <<
    m_cpt_rgb[i].m_color.y << " " <<
    m_cpt_rgb[i].m_color.z << " " <<
    m_cpt_rgb[i].m_isoValue << " " << "\n";
    }
    myfile << (int)m_cpt_alpha.size () << "\n";
    for (int i = 0; i < (int)m_cpt_alpha.size (); i++)
    {
    myfile << m_cpt_alpha[i].m_color.w << " " <<
    m_cpt_alpha[i].m_isoValue << " " << "\n";
    }
    myfile.close ();
    printf ("lqc: Transfer Function 1D Control Points Saved!\n");
    }
    else
    {
    printf ("lqc: Error on opening file at VRTransferFunction::Save().\n");
    }
    }
    */
    
    return true;
  }

  bool TransferFunction2D::Load ()//std::string filename, TFFormatType format)
  {
    /*
    std::string filesaved;
    filesaved.append (RESOURCE_LIBLQC_PATH);
    filesaved.append ("TransferFunctions/");
    filesaved.append (filename);
    if (format == TFFormatType::LQC)
    {
    filesaved.append (".tf1d");
    std::ifstream myfile (filesaved.c_str ());
    if (myfile.is_open ())
    {
    int init;
    myfile >> init;

    int cpt_rgb_size;
    myfile >> cpt_rgb_size;
    float r, g, b, a;
    int isovalue;
    for (int i = 0; i < cpt_rgb_size; i++)
    {
    myfile >> r >> g >> b >> isovalue;
    m_cpt_rgb.push_back (TransferControlPoint (r, g, b, isovalue));
    }

    int cpt_alpha_size;
    myfile >> cpt_alpha_size;
    for (int i = 0; i < cpt_alpha_size; i++)
    {
    myfile >> a >> isovalue;
    m_cpt_alpha.push_back (TransferControlPoint (a, isovalue));
    }
    myfile.close ();
    printf ("lqc: Transfer Function 1D Control Points Loaded!\n");
    return true;
    }
    else
    printf ("lqc: Error on opening file at VRTransferFunction::AddControlPointsReadFile().\n");
    }
    return false;
    */

    return true;
  }

  void TransferFunction2D::BuildLinear ()
  {
    for (int i = 0; i < (int)m_cpt_rgb.size() - 1; i++)
    {
      int i0 = m_cpt_rgb[i].m_isoValue;
      int i1 = m_cpt_rgb[i + 1].m_isoValue;

      glm::dvec3 diff = glm::dvec3(
        m_cpt_rgb[i + 1].m_color.r - m_cpt_rgb[i].m_color.r,
        m_cpt_rgb[i + 1].m_color.g - m_cpt_rgb[i].m_color.g,
        m_cpt_rgb[i + 1].m_color.b - m_cpt_rgb[i].m_color.b
      );

      for (int x = i0; x <= i1; x++)
      {
        double k = (double)(x - i0) / (double)(i1 - i0);

        m_transferfunction[x].r = m_cpt_rgb[i].m_color.r + diff.r * k;
        m_transferfunction[x].g = m_cpt_rgb[i].m_color.g + diff.g * k;
        m_transferfunction[x].b = m_cpt_rgb[i].m_color.b + diff.b * k;
      }
    }

    for (int i = 0; i < (int)m_cpt_alpha.size() - 1; i++)
    {
      int i0 = m_cpt_alpha[i].m_isoValue;
      int i1 = m_cpt_alpha[i + 1].m_isoValue;

      double diff = double(
        m_cpt_alpha[i + 1].m_color.a - m_cpt_alpha[i].m_color.a
        );

      for (int x = i0; x <= i1; x++)
      {
        double k = (double)(x - i0) / (double)(i1 - i0);

        m_transferfunction[x].a = m_cpt_alpha[i].m_color.a + diff * k;
      }
    }
  }
}