/*
 * Software License Agreement (BSD License)
 *
 *  Copyright (c) 2010, Willow Garage, Inc.
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above
 *     copyright notice, this list of conditions and the following
 *     disclaimer in the documentation and/or other materials provided
 *     with the distribution.
 *   * Neither the name of Willow Garage, Inc. nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 *  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 *  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 *  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 *  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 *  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 *  ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 *
 * $Id: point_cloud_handlers.cpp 36250 2011-02-25 19:57:32Z rusu $
 *
 */

#include <pcl_visualization/point_cloud_handlers.h>
#include <terminal_tools/time.h>
#include <pcl/win32_macros.h>

//////////////////////////////////////////////////////////////////////////////////////////////
/** \brief Obtain the actual color for the input dataset as vtk scalars.
  * \param scalars the resultant scalars containing the color for the input dataset
  */
void
pcl_visualization::PointCloudColorHandlerCustom<sensor_msgs::PointCloud2>::getColor (vtkSmartPointer<vtkDataArray> &scalars) const
{
  if (!capable_)
    return;

  if (!scalars)
    scalars = vtkSmartPointer<vtkUnsignedCharArray>::New ();
  scalars->SetNumberOfComponents (3);
  
  vtkIdType nr_points = cloud_->width * cloud_->height;
  reinterpret_cast<vtkUnsignedCharArray*>(&(*scalars))->SetNumberOfTuples (nr_points);

  // Get a random color
  unsigned char* colors = new unsigned char[nr_points * 3];

  // Color every point
  for (vtkIdType cp = 0; cp < nr_points; ++cp)
  {
    colors[cp * 3 + 0] = r_;
    colors[cp * 3 + 1] = g_;
    colors[cp * 3 + 2] = b_;
  }
  reinterpret_cast<vtkUnsignedCharArray*>(&(*scalars))->SetArray (colors, 3 * nr_points, 0);
}

//////////////////////////////////////////////////////////////////////////////////////////////
/** \brief Obtain the actual color for the input dataset as vtk scalars.
  * \param scalars the resultant scalars containing the color for the input dataset
  */
void
pcl_visualization::PointCloudColorHandlerRandom<sensor_msgs::PointCloud2>::getColor (vtkSmartPointer<vtkDataArray> &scalars) const
{
  if (!capable_)
    return;

  if (!scalars)
    scalars = vtkSmartPointer<vtkUnsignedCharArray>::New ();
  scalars->SetNumberOfComponents (3);
  
  vtkIdType nr_points = cloud_->width * cloud_->height;
  reinterpret_cast<vtkUnsignedCharArray*>(&(*scalars))->SetNumberOfTuples (nr_points);

  // Get a random color
  unsigned char* colors = new unsigned char[nr_points * 3];
  double r, g, b;
  pcl_visualization::getRandomColors (r, g, b);

  int r_ = lrint (r * 255.0), g_ = lrint (g * 255.0), b_ = lrint (b * 255.0);

  // Color every point
  for (vtkIdType cp = 0; cp < nr_points; ++cp)
  {
    colors[cp * 3 + 0] = r_;
    colors[cp * 3 + 1] = g_;
    colors[cp * 3 + 2] = b_;
  }
  reinterpret_cast<vtkUnsignedCharArray*>(&(*scalars))->SetArray (colors, 3 * nr_points, 0);
}

//////////////////////////////////////////////////////////////////////////////////////////////
/** \brief Constructor. */
pcl_visualization::PointCloudColorHandlerRGBField<sensor_msgs::PointCloud2>::PointCloudColorHandlerRGBField (
    const pcl_visualization::PointCloudColorHandler<sensor_msgs::PointCloud2>::PointCloud &cloud) : 
  pcl_visualization::PointCloudColorHandler<sensor_msgs::PointCloud2>::PointCloudColorHandler (cloud)
{
  // Handle the 24-bit packed RGB values
  field_idx_ = pcl::getFieldIndex (cloud, "rgb");
  if (field_idx_ != -1)
    capable_ = true;
  else
    capable_ = false;
}

//////////////////////////////////////////////////////////////////////////////////////////////
/** \brief Obtain the actual color for the input dataset as vtk scalars.
  * \param scalars the resultant scalars containing the color for the input dataset
  */
void 
pcl_visualization::PointCloudColorHandlerRGBField<sensor_msgs::PointCloud2>::getColor (vtkSmartPointer<vtkDataArray> &scalars) const
{
  if (!capable_)
    return;

  if (!scalars)
    scalars = vtkSmartPointer<vtkUnsignedCharArray>::New ();
  scalars->SetNumberOfComponents (3);

  vtkIdType nr_points = cloud_->width * cloud_->height;
  reinterpret_cast<vtkUnsignedCharArray*>(&(*scalars))->SetNumberOfTuples (nr_points);
  
  
  // Allocate enough memory to hold all colors
  unsigned char* colors = new unsigned char[nr_points * 3];

  float rgb_data;
  int point_offset = cloud_->fields[field_idx_].offset;
  int j = 0;
  
  // If XYZ present, check if the points are invalid
  int x_idx = pcl::getFieldIndex (*cloud_, "x");
  if (x_idx != -1)
  {
    float x_data;
    int x_point_offset = cloud_->fields[x_idx].offset;
    
    // Color every point
    for (vtkIdType cp = 0; cp < nr_points; ++cp, 
                                           point_offset += cloud_->point_step, 
                                           x_point_offset += cloud_->point_step)
    {
      // Copy the value at the specified field
      memcpy (&rgb_data, &cloud_->data[point_offset], sizeof (float));

      if (!pcl_isfinite (rgb_data))
        continue;

      memcpy (&x_data, &cloud_->data[x_point_offset], sizeof (float));
      if (!pcl_isfinite (x_data))
        continue;

      int rgb = *reinterpret_cast<int*>(&rgb_data);
      colors[j * 3 + 0] = ((rgb >> 16) & 0xff);
      colors[j * 3 + 1] = ((rgb >> 8) & 0xff);
      colors[j * 3 + 2] = (rgb & 0xff);
      j++;
    }
  }
  // No XYZ data checks
  else
  {
    // Color every point
    for (vtkIdType cp = 0; cp < nr_points; ++cp, point_offset += cloud_->point_step)
    {
      // Copy the value at the specified field
      memcpy (&rgb_data, &cloud_->data[point_offset], sizeof (float));

      if (!pcl_isfinite (rgb_data))
        continue;

      int rgb = *reinterpret_cast<int*>(&rgb_data);
      colors[j * 3 + 0] = ((rgb >> 16) & 0xff);
      colors[j * 3 + 1] = ((rgb >> 8) & 0xff);
      colors[j * 3 + 2] = (rgb & 0xff);
      j++;
    }
  }
  reinterpret_cast<vtkUnsignedCharArray*>(&(*scalars))->SetArray (colors, 3 * j, 0);
  //delete [] colors;
}

//////////////////////////////////////////////////////////////////////////////////////////////
/** \brief Constructor. */
pcl_visualization::PointCloudColorHandlerGenericField<sensor_msgs::PointCloud2>::PointCloudColorHandlerGenericField (
    const pcl_visualization::PointCloudColorHandler<sensor_msgs::PointCloud2>::PointCloud &cloud, 
    const std::string &field_name) : pcl_visualization::PointCloudColorHandler<sensor_msgs::PointCloud2>::PointCloudColorHandler (cloud)
{
  field_name_ = field_name;
  field_idx_  = pcl::getFieldIndex (cloud, field_name);
  if (field_idx_ != -1)
    capable_ = true;
  else
    capable_ = false;
}

//////////////////////////////////////////////////////////////////////////////////////////////
/** \brief Obtain the actual color for the input dataset as vtk scalars.
  * \param scalars the resultant scalars containing the color for the input dataset
  */
void 
pcl_visualization::PointCloudColorHandlerGenericField<sensor_msgs::PointCloud2>::getColor (vtkSmartPointer<vtkDataArray> &scalars) const
{
  if (!capable_)
    return;

  if (!scalars)
    scalars = vtkSmartPointer<vtkFloatArray>::New ();
  scalars->SetNumberOfComponents (1);

  vtkIdType nr_points = cloud_->width * cloud_->height;
  reinterpret_cast<vtkFloatArray*>(&(*scalars))->SetNumberOfTuples (nr_points);

  float* colors = new float[nr_points];
  float field_data;
  int j = 0;
  int point_offset = cloud_->fields[field_idx_].offset;

  // If XYZ present, check if the points are invalid
  int x_idx = pcl::getFieldIndex (*cloud_, "x");
  if (x_idx != -1)
  {
    float x_data;
    int x_point_offset = cloud_->fields[x_idx].offset;
    
    // Color every point
    for (vtkIdType cp = 0; cp < nr_points; ++cp,
                                           point_offset += cloud_->point_step, 
                                           x_point_offset += cloud_->point_step)
    {
      // Copy the value at the specified field
      memcpy (&field_data, &cloud_->data[point_offset], sizeof (float));

      if (!pcl_isfinite (field_data))
        continue;

      memcpy (&x_data, &cloud_->data[x_point_offset], sizeof (float));
      if (!pcl_isfinite (x_data))
        continue;

      colors[j] = field_data;
      j++;
    }
  }
  // No XYZ data checks
  else
  {
    // Color every point
    for (vtkIdType cp = 0; cp < nr_points; ++cp, point_offset += cloud_->point_step)
    {
      // Copy the value at the specified field
      memcpy (&field_data, &cloud_->data[point_offset], sizeof (float));

      if (!pcl_isfinite (field_data))
        continue;
      colors[j] = field_data;
      j++;
    }
  }
  reinterpret_cast<vtkFloatArray*>(&(*scalars))->SetArray (colors, j, 0);
}

//////////////////////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////////////////////
/** \brief Obtain the actual point geometry for the input dataset as a vtk pointset.
  * \param points the resultant geometry 
  */
void 
pcl_visualization::PointCloudGeometryHandler<sensor_msgs::PointCloud2>::getGeometry (vtkSmartPointer<vtkPoints> &points) const
{
  if (!capable_)
    return;

  if (!points)
    points = vtkSmartPointer<vtkPoints>::New ();
  points->SetDataTypeToFloat ();

  vtkSmartPointer<vtkFloatArray> data = vtkSmartPointer<vtkFloatArray>::New ();
  data->SetNumberOfComponents (3);
  vtkIdType nr_points = cloud_->width * cloud_->height;

  // Add all points
  float dim;
  vtkIdType j = 0;    // true point index
  float* pts = new float[nr_points * 3];
  int point_offset = 0;

  // If the dataset has no invalid values, just copy all of them
  if (cloud_->is_dense)
  {
    for (vtkIdType i = 0; i < nr_points; ++i, point_offset+=cloud_->point_step)
    {
      // Copy the value at the specified field
      memcpy (&dim, &cloud_->data[point_offset + cloud_->fields[field_x_idx_].offset], sizeof (float));
      pts[i * 3 + 0] = dim;

      memcpy (&dim, &cloud_->data[point_offset + cloud_->fields[field_y_idx_].offset], sizeof (float));
      pts[i * 3 + 1] = dim;

      memcpy (&dim, &cloud_->data[point_offset + cloud_->fields[field_z_idx_].offset], sizeof (float));
      pts[i * 3 + 2] = dim;
    }
    data->SetArray (&pts[0], nr_points * 3, 0);
    points->SetData (data);
  }
  else
  {
    for (vtkIdType i = 0; i < nr_points; ++i, point_offset+=cloud_->point_step)
    {
      // Copy the value at the specified field
      memcpy (&dim, &cloud_->data[point_offset + cloud_->fields[field_x_idx_].offset], sizeof (float));
      if (!pcl_isfinite (dim))
        continue;
      pts[j * 3 + 0] = dim;

      memcpy (&dim, &cloud_->data[point_offset + cloud_->fields[field_y_idx_].offset], sizeof (float));
      if (!pcl_isfinite (dim))
        continue;
      pts[j * 3 + 1] = dim;

      memcpy (&dim, &cloud_->data[point_offset + cloud_->fields[field_z_idx_].offset], sizeof (float));
      if (!pcl_isfinite (dim))
        continue;
      pts[j * 3 + 2] = dim;

      // Set j and increment
      j++;
    }
    data->SetArray (&pts[0], j * 3, 0);
    points->SetData (data);
  }
}

//////////////////////////////////////////////////////////////////////////////////////////////
/** \brief Constructor. */
pcl_visualization::PointCloudGeometryHandlerXYZ<sensor_msgs::PointCloud2>::PointCloudGeometryHandlerXYZ (const PointCloud &cloud)
: pcl_visualization::PointCloudGeometryHandler<sensor_msgs::PointCloud2>::PointCloudGeometryHandler (cloud)
{
  field_x_idx_ = pcl::getFieldIndex (cloud, "x");
  if (field_x_idx_ == -1)
    return;
  field_y_idx_ = pcl::getFieldIndex (cloud, "y");
  if (field_y_idx_ == -1)
    return;
  field_z_idx_ = pcl::getFieldIndex (cloud, "z");
  if (field_z_idx_ == -1)
    return;
  capable_ = true;
}



//////////////////////////////////////////////////////////////////////////////////////////////
/** \brief Constructor. */
pcl_visualization::PointCloudGeometryHandlerSurfaceNormal<sensor_msgs::PointCloud2>::PointCloudGeometryHandlerSurfaceNormal (const PointCloud &cloud) 
: pcl_visualization::PointCloudGeometryHandler<sensor_msgs::PointCloud2>::PointCloudGeometryHandler (cloud)
{
  field_x_idx_ = pcl::getFieldIndex (cloud, "normal_x");
  if (field_x_idx_ == -1)
    return;
  field_y_idx_ = pcl::getFieldIndex (cloud, "normal_y");
  if (field_y_idx_ == -1)
    return;
  field_z_idx_ = pcl::getFieldIndex (cloud, "normal_z");
  if (field_z_idx_ == -1)
    return;
  capable_ = true;
}

//////////////////////////////////////////////////////////////////////////////////////////////
/** \brief Constructor. */
pcl_visualization::PointCloudGeometryHandlerCustom<sensor_msgs::PointCloud2>::PointCloudGeometryHandlerCustom (
    const PointCloud &cloud, const std::string &x_field_name, const std::string &y_field_name, const std::string &z_field_name) 
: pcl_visualization::PointCloudGeometryHandler<sensor_msgs::PointCloud2>::PointCloudGeometryHandler (cloud)
{
  field_x_idx_ = pcl::getFieldIndex (cloud, x_field_name);
  if (field_x_idx_ == -1)
    return;
  field_y_idx_ = pcl::getFieldIndex (cloud, y_field_name);
  if (field_y_idx_ == -1)
    return;
  field_z_idx_ = pcl::getFieldIndex (cloud, z_field_name);
  if (field_z_idx_ == -1)
    return;
  field_name_ = x_field_name + y_field_name + z_field_name;
  capable_ = true;
}

