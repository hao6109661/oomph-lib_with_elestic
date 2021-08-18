// LIC// ====================================================================
// LIC// This file forms part of oomph-lib, the object-oriented,
// LIC// multi-physics finite-element library, available
// LIC// at http://www.oomph-lib.org.
// LIC//
// LIC// Copyright (C) 2006-2021 Matthias Heil and Andrew Hazel
// LIC//
// LIC// This library is free software; you can redistribute it and/or
// LIC// modify it under the terms of the GNU Lesser General Public
// LIC// License as published by the Free Software Foundation; either
// LIC// version 2.1 of the License, or (at your option) any later version.
// LIC//
// LIC// This library is distributed in the hope that it will be useful,
// LIC// but WITHOUT ANY WARRANTY; without even the implied warranty of
// LIC// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// LIC// Lesser General Public License for more details.
// LIC//
// LIC// You should have received a copy of the GNU Lesser General Public
// LIC// License along with this library; if not, write to the Free Software
// LIC// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
// LIC// 02110-1301  USA.
// LIC//
// LIC// The authors may be contacted at oomph-lib@maths.man.ac.uk.
// LIC//
// LIC//====================================================================
// Header file for a class that is used to represent a mesh
// as a geometric object

// Include guards to prevent multiple inclusion of the header
#ifndef OOMPH_MESH_AS_GEOMETRIC_OBJECT_HEADER
#define OOMPH_MESH_AS_GEOMETRIC_OBJECT_HEADER

// Config header generated by autoconfig
#ifdef HAVE_CONFIG_H
#include <oomph-lib-config.h>
#endif

#include <float.h>
#include <limits.h>

// Include the geometric object header file
#include "geom_objects.h"

// Sample point container
#include "sample_point_container.h"


#include "sample_point_parameters.h"

namespace oomph
{
  ////////////////////////////////////////////////////////////////////////
  ////////////////////////////////////////////////////////////////////////
  ////////////////////////////////////////////////////////////////////////

  //========================================================================
  /// Helper namespace for MeshAsGeomObject -- its only function creates
  /// SamplePointContainerParameters of the right type for the default sample
  /// point container
  //========================================================================
  namespace MeshAsGeomObject_Helper
  {
    /// Default sample point container type
    extern unsigned Default_sample_point_container_version;

    /// \short "Factory" for SamplePointContainerParameters of the right type as
    /// selected by Default_sample_point_container_version
    extern void create_sample_point_container_parameters(
      Mesh* mesh_pt,
      SamplePointContainerParameters*& sample_point_container_parameters_pt);

  } // namespace MeshAsGeomObject_Helper


  ////////////////////////////////////////////////////////////////////////
  ////////////////////////////////////////////////////////////////////////
  ////////////////////////////////////////////////////////////////////////


  //========================================================================
  /// This class provides a GeomObject representation of a given
  /// finite element mesh. The Lagrangian coordinate is taken to be the
  /// dimension of the (first) element in the mesh and the Eulerian
  /// coordinate is taken to be the dimension of the (first) node in
  /// the mesh. If there are no elements or nodes the appropriate dimensions
  /// will be set to zero.
  /// The constituent elements of the mesh must have their own
  /// GeomObject representations, so they must be FiniteElements,
  ///  and they become sub-objects
  /// in this compound GeomObject.
  //========================================================================
  class MeshAsGeomObject : public GeomObject
  {
  private:
    /// Helper function to actually build the thing
    void build_it(
      SamplePointContainerParameters* sample_point_container_parameters_pt)
    {
      Mesh_pt = sample_point_container_parameters_pt->mesh_pt();
      if (dynamic_cast<RefineableBinArrayParameters*>(
            sample_point_container_parameters_pt) != 0)
      {
        Sample_point_container_version = UseRefineableBinArray;
      }
      else if (dynamic_cast<NonRefineableBinArrayParameters*>(
                 sample_point_container_parameters_pt) != 0)
      {
        Sample_point_container_version = UseNonRefineableBinArray;
      }
#ifdef OOMPH_HAS_CGAL
      else if (dynamic_cast<CGALSamplePointContainerParameters*>(
                 sample_point_container_parameters_pt) != 0)
      {
        Sample_point_container_version = UseCGALSamplePointContainer;
      }
#endif
      else
      {
        throw OomphLibError("Wrong sample_point_container_parameters_pt",
                            OOMPH_CURRENT_FUNCTION,
                            OOMPH_EXCEPTION_LOCATION);
      }

#ifdef OOMPH_HAS_MPI

      // Set communicator
      Communicator_pt = Mesh_pt->communicator_pt();

#endif


      // Storage for the Lagrangian and Eulerian dimension
      int dim[2] = {0, 0};

      // Set the Lagrangian dimension from the dimension of the first element
      // if it exists (if not the Lagrangian dimension will be zero)
      if (Mesh_pt->nelement() != 0)
      {
        dim[0] = Mesh_pt->finite_element_pt(0)->dim();
      }

      // Read out the Eulerian dimension from the first node, if it exists.
      //(if not the Eulerian dimension will be zero);
      if (Mesh_pt->nnode() != 0)
      {
        dim[1] = Mesh_pt->node_pt(0)->ndim();
      }

      // Need to do an Allreduce to ensure that the dimension is consistent
      // even when no elements are assigned to a certain processor
#ifdef OOMPH_HAS_MPI

      // Only a problem if the mesh has been distributed
      if (Mesh_pt->is_mesh_distributed())
      {
        // Need a non-null communicator
        if (Communicator_pt != 0)
        {
          int n_proc = Communicator_pt->nproc();
          if (n_proc > 1)
          {
            int dim_reduce[2];
            MPI_Allreduce(&dim,
                          &dim_reduce,
                          2,
                          MPI_INT,
                          MPI_MAX,
                          Communicator_pt->mpi_comm());

            dim[0] = dim_reduce[0];
            dim[1] = dim_reduce[1];
          }
        }
      }
#endif

      // Set the Lagrangian and Eulerian dimensions within this geometric object
      this->set_nlagrangian_and_ndim(static_cast<unsigned>(dim[0]),
                                     static_cast<unsigned>(dim[1]));

      // Create temporary storage for geometric Data (don't count
      // Data twice!
      std::set<Data*> tmp_geom_data;

      // Copy all the elements in the mesh into local storage
      // N.B. elements must be able to have a geometric object representation.
      unsigned n_sub_object = Mesh_pt->nelement();
      Sub_geom_object_pt.resize(n_sub_object);
      for (unsigned e = 0; e < n_sub_object; e++)
      {
        // (Try to) cast to a finite element:
        Sub_geom_object_pt[e] =
          dynamic_cast<FiniteElement*>(Mesh_pt->element_pt(e));

#ifdef PARANOID
        if (Sub_geom_object_pt[e] == 0)
        {
          std::ostringstream error_message;
          error_message << "Unable to dynamic cast element: " << std::endl
                        << "into a FiniteElement: GeomObject representation is "
                           "not possible\n";
          throw OomphLibError(error_message.str(),
                              OOMPH_CURRENT_FUNCTION,
                              OOMPH_EXCEPTION_LOCATION);
        }
#endif

        // Add the geometric Data of each element into set
        unsigned ngeom_data = Sub_geom_object_pt[e]->ngeom_data();
        for (unsigned i = 0; i < ngeom_data; i++)
        {
          tmp_geom_data.insert(Sub_geom_object_pt[e]->geom_data_pt(i));
        }
      }

      // Now copy unique geom Data values across into vector
      unsigned ngeom = tmp_geom_data.size();
      Geom_data_pt.resize(ngeom);
      typedef std::set<Data*>::iterator IT;
      unsigned count = 0;
      for (IT it = tmp_geom_data.begin(); it != tmp_geom_data.end(); it++)
      {
        Geom_data_pt[count] = *it;
        count++;
      }

      // Build the right type of bin array
      switch (Sample_point_container_version)
      {
        case UseRefineableBinArray:

          Sample_point_container_pt =
            new RefineableBinArray(sample_point_container_parameters_pt);
          break;

        case UseNonRefineableBinArray:

          Sample_point_container_pt =
            new NonRefineableBinArray(sample_point_container_parameters_pt);
          break;

#ifdef OOMPH_HAS_CGAL

        case UseCGALSamplePointContainer:

          Sample_point_container_pt =
            new CGALSamplePointContainer(sample_point_container_parameters_pt);
          break;

#endif

        default:

          oomph_info << "Sample_point_container_version = "
                     << Sample_point_container_version << std::endl;
          throw OomphLibError("Sample_point_container_version",
                              OOMPH_CURRENT_FUNCTION,
                              OOMPH_EXCEPTION_LOCATION);
      }
    }


    /// \short Vector of pointers to Data items that affects the object's shape
    Vector<Data*> Geom_data_pt;

    /// Internal storage for the elements that constitute the object
    Vector<FiniteElement*> Sub_geom_object_pt;

    /// \short Pointer to the sample point container
    SamplePointContainer* Sample_point_container_pt;

#ifdef OOMPH_HAS_MPI

    /// Communicator
    OomphCommunicator* Communicator_pt;

#endif

    /// Pointer to mesh
    Mesh* Mesh_pt;

    /// \short Which version of the sample point container
    /// are we using?
    unsigned Sample_point_container_version;

  public:
    /// \short Pointer to the sample point container
    SamplePointContainer* sample_point_container_pt() const
    {
      return Sample_point_container_pt;
    }

    /// Return pointer to e-th finite element
    FiniteElement* finite_element_pt(const unsigned& e)
    {
      return Sub_geom_object_pt[e];
    }


    /// \short Which sample point container is used in locate zeta? (uses enum
    /// Sample_Point_Container_Type)
    unsigned sample_point_container_version() const
    {
      return Sample_point_container_version;
    }

    /// Number of elements in the underlying mesh
    unsigned nelement()
    {
      return Sub_geom_object_pt.size();
    }

    ///\short Constructor
    MeshAsGeomObject(Mesh* const& mesh_pt) : GeomObject()
    {
      // Create default parameters
      SamplePointContainerParameters* sample_point_container_parameters_pt = 0;
      MeshAsGeomObject_Helper::create_sample_point_container_parameters(
        mesh_pt, sample_point_container_parameters_pt);

      // Build the bastard
      build_it(sample_point_container_parameters_pt);
      delete sample_point_container_parameters_pt;
    }


    ///\short Constructor
    MeshAsGeomObject(
      SamplePointContainerParameters* sample_point_container_parameters_pt)
      : GeomObject()
    {
      build_it(sample_point_container_parameters_pt);
    }

    /// Empty Constructor
    MeshAsGeomObject() {}

    /// Destructor
    ~MeshAsGeomObject()
    {
      delete Sample_point_container_pt;
    }

    /// Broken copy constructor
    MeshAsGeomObject(const MeshAsGeomObject&)
    {
      BrokenCopy::broken_copy("MeshAsGeomObject");
    }

    /// Broken assignment operator
    void operator=(const MeshAsGeomObject&)
    {
      BrokenCopy::broken_assign("MeshAsGeomObject");
    }

    /// How many items of Data does the shape of the object depend on?
    unsigned ngeom_data() const
    {
      return Geom_data_pt.size();
    }

    /// \short Return pointer to the j-th Data item that the object's
    /// shape depends on
    Data* geom_data_pt(const unsigned& j)
    {
      return Geom_data_pt[j];
    }

    /// \short Find the sub geometric object and local coordinate therein that
    /// corresponds to the intrinsic coordinate zeta. If sub_geom_object_pt=0
    /// on return from this function, none of the constituent sub-objects
    /// contain the required coordinate. Following from the general
    /// interface to this function in GeomObjects,
    /// setting the optional bool argument to true means that each
    /// time the sub-object's locate_zeta function is called, the coordinate
    /// argument "s" is used as the initial guess. However, this doesn't
    /// make sense here and the argument is ignored (though a warning
    /// is issued when the code is compiled in PARANOID setting)
    void locate_zeta(const Vector<double>& zeta,
                     GeomObject*& sub_geom_object_pt,
                     Vector<double>& s,
                     const bool& use_coordinate_as_initial_guess = false)
    {
#ifdef PARANOID
      if (use_coordinate_as_initial_guess)
      {
        OomphLibWarning(
          "Ignoring the use_coordinate_as_initial_guess argument.",
          "MeshAsGeomObject::locate_zeta()",
          OOMPH_EXCEPTION_LOCATION);
      }
#endif


      // Do locate in sample point container
      Sample_point_container_pt->locate_zeta(zeta, sub_geom_object_pt, s);
    }

    /// \short Return the position as a function of the intrinsic coordinate
    /// zeta. This provides an (expensive!) default implementation in which we
    /// loop over all the constituent sub-objects and check if they contain zeta
    /// and then evaluate their position() function.
    void position(const Vector<double>& zeta, Vector<double>& r) const
    {
      // Call position function at current timestep:
      unsigned t = 0;
      position(t, zeta, r);
    }

    /// \short Parametrised position on object: r(zeta). Evaluated at
    /// previous timestep. t=0: current time; t>0: previous
    /// timestep. This provides an (expensive!) default implementation in which
    /// we loop over all the constituent sub-objects and check if they
    /// contain zeta and then evaluate their position() function.
    void position(const unsigned& t,
                  const Vector<double>& zeta,
                  Vector<double>& r) const
    {
      // Storage for the GeomObject that contains the zeta coordinate
      // and the intrinsic coordinate within it.
      GeomObject* sub_geom_object_pt;
      const unsigned n_lagrangian = this->nlagrangian();
      Vector<double> s(n_lagrangian);

      // Find the sub object containing zeta, and the local intrinsic coordinate
      // within it
      const_cast<MeshAsGeomObject*>(this)->locate_zeta(
        zeta, sub_geom_object_pt, s);
      if (sub_geom_object_pt == 0)
      {
        std::ostringstream error_message;
        error_message << "Cannot locate zeta ";
        for (unsigned i = 0; i < n_lagrangian; i++)
        {
          error_message << zeta[i] << " ";
        }
        error_message << std::endl;
        Mesh_pt->output("most_recent_mesh.dat");
        throw OomphLibError(error_message.str(),
                            OOMPH_CURRENT_FUNCTION,
                            OOMPH_EXCEPTION_LOCATION);
      }
      // Call that sub-object's position function
      sub_geom_object_pt->position(t, s, r);

    } // end of position

    /// Return the derivative of the position
    void dposition(const Vector<double>& xi, DenseMatrix<double>& drdxi) const
    {
      throw OomphLibError("dposition() not implemented",
                          OOMPH_CURRENT_FUNCTION,
                          OOMPH_EXCEPTION_LOCATION);
    }
  };

} // namespace oomph

#endif
