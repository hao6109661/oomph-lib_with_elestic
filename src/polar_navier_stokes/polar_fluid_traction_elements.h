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
// Header file for elements that are used to integrate fluid tractions
// This includes the guts (i.e. equations) because we want to inline them
// for faster operation, although it slows down the compilation!

#ifndef OOMPH_POLAR_FLUID_TRACTION_ELEMENTS_HEADER
#define OOMPH_POLAR_FLUID_TRACTION_ELEMENTS_HEADER

// Config header generated by autoconfig
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif


// OOMPH-LIB headers
#include "../generic/Qelements.h"

namespace oomph
{
  //======================================================================
  /// A class for elements that allow the imposition of an applied traction
  /// to the Navier--Stokes equations
  /// The geometrical information can be read from the FaceGeometery<ELEMENT>
  /// class and and thus, we can be generic enough without the need to have
  /// a separate equations class
  //======================================================================
  template<class ELEMENT>
  class PolarNavierStokesTractionElement : public virtual FaceGeometry<ELEMENT>,
                                           public virtual FaceElement
  {
  private:
    /// Pointer to an imposed traction function
    void (*Traction_fct_pt)(const double& time,
                            const Vector<double>& x,
                            Vector<double>& result);

    /// The highest dimension of the problem
    unsigned Dim;

  protected:
    /// Access function that returns the local equation numbers
    /// for velocity components.
    /// u_local_eqn(n,i) = local equation number or < 0 if pinned.
    /// The default is to asssume that n is the local node number
    /// and the i-th velocity component is the i-th unknown stored at the node.
    virtual inline int u_local_eqn(const unsigned& n, const unsigned& i)
    {
      return nodal_local_eqn(n, i);
    }

    /// Function to compute the shape and test functions and to return
    /// the Jacobian of mapping
    inline double shape_and_test_at_knot(const unsigned& ipt,
                                         Shape& psi,
                                         Shape& test) const
    {
      // Find number of nodes
      unsigned n_node = nnode();
      // Calculate the shape functions
      shape_at_knot(ipt, psi);
      // Set the test functions to be the same as the shape functions
      for (unsigned i = 0; i < n_node; i++)
      {
        test[i] = psi[i];
      }
      // Return the value of the jacobian
      return J_eulerian_at_knot(ipt);
    }


    /// Function to calculate the traction applied to the fluid
    void get_traction(double time,
                      const Vector<double>& x,
                      Vector<double>& result)
    {
      // If the function pointer is zero return zero
      if (Traction_fct_pt == 0)
      {
        // Loop over dimensions and set body forces to zero
        for (unsigned i = 0; i < Dim; i++)
        {
          result[i] = 0.0;
        }
      }
      // Otherwise call the function
      else
      {
        (*Traction_fct_pt)(time, x, result);
      }
    }

    /// This function returns the residuals for the
    /// traction function.
    /// flag=1(or 0): do (or don't) compute the Jacobian as well.
    void fill_in_generic_residual_contribution(Vector<double>& residuals,
                                               DenseMatrix<double>& jacobian,
                                               DenseMatrix<double>& mass_matrix,
                                               unsigned flag);
    /// Pointer to the angle alpha
    double* Alpha_pt;

    /// Pointer to the Data item that stores the external pressure
    Data* Pext_data_pt;

    /// The Data that contains the traded pressure is stored
    /// as external Data for the element. Which external Data item is it?
    unsigned External_data_number_of_Pext;

    // Traction elements need to know whether they're at the inlet or outlet
    // as the unit outward normal has a differing sign dependent on
    // the boundary
    // -1=inlet, 1=outlet
    int Boundary;

    // Pointer to homotopy parameter
    double Eta;

  public:
    /// Alpha
    const double& alpha() const
    {
      return *Alpha_pt;
    }

    /// Pointer to Alpha
    double*& alpha_pt()
    {
      return Alpha_pt;
    }

    /// Function for setting up external pressure
    void set_external_pressure_data(Data* pext_data_pt)
    {
      // Set external pressure pointer
      Pext_data_pt = pext_data_pt;

      // Add to the element's external data so it gets included
      // in the black-box local equation numbering scheme
      External_data_number_of_Pext = this->add_external_data(pext_data_pt);
    }

    /// Boundary
    const int boundary() const
    {
      return Boundary;
    }

    /// Function to set boundary
    void set_boundary(int bound)
    {
      Boundary = bound;
    }

    /// Eta
    const double get_eta() const
    {
      return Eta;
    }

    /// Function to set Eta
    void set_eta(double eta)
    {
      Eta = eta;
    }

    /// Constructor, which takes a "bulk" element and the value of the index
    /// and its limit
    PolarNavierStokesTractionElement(FiniteElement* const& element_pt,
                                     const int& face_index)
      : FaceGeometry<ELEMENT>(), FaceElement()
    {
      // Attach the geometrical information to the element. N.B. This function
      // also assigns nbulk_value from the required_nvalue of the bulk element
      element_pt->build_face_element(face_index, this);

#ifdef PARANOID
      {
        // Check that the element is not a refineable 3d element
        ELEMENT* elem_pt = dynamic_cast<ELEMENT*>(element_pt);
        // If it's three-d
        if (elem_pt->dim() == 3)
        {
          // Is it refineable
          RefineableElement* ref_el_pt =
            dynamic_cast<RefineableElement*>(elem_pt);
          if (ref_el_pt != 0)
          {
            if (this->has_hanging_nodes())
            {
              throw OomphLibError("This flux element will not work correctly "
                                  "if nodes are hanging\n",
                                  OOMPH_CURRENT_FUNCTION,
                                  OOMPH_EXCEPTION_LOCATION);
            }
          }
        }
      }
#endif

      // Set the body force function pointer to zero
      Traction_fct_pt = 0;

      // Set the external pressure pointer to be zero
      Pext_data_pt = 0;

      // Set the dimension from the dimension of the first node
      Dim = this->node_pt(0)->ndim();

      // Set Eta to one by default
      Eta = 1.0;
    }

    /// Destructor should not delete anything
    ~PolarNavierStokesTractionElement() {}

    // Access function for the imposed traction pointer
    void (*&traction_fct_pt())(const double& t,
                               const Vector<double>& x,
                               Vector<double>& result)
    {
      return Traction_fct_pt;
    }

    /// This function returns just the residuals
    inline void fill_in_contribution_to_residuals(Vector<double>& residuals)
    {
      // Call the generic residuals function with flag set to 0
      // using a dummy matrix argument
      fill_in_generic_residual_contribution(residuals,
                                            GeneralisedElement::Dummy_matrix,
                                            GeneralisedElement::Dummy_matrix,
                                            0);
    }

    /// This function returns the residuals and the jacobian
    inline void fill_in_contribution_to_jacobian(Vector<double>& residuals,
                                                 DenseMatrix<double>& jacobian)
    {
      // Call the generic routine with the flag set to 1
      fill_in_generic_residual_contribution(
        residuals, jacobian, GeneralisedElement::Dummy_matrix, 1);
    }

    /// Compute the element's residual Vector and the jacobian matrix
    /// Plus the mass matrix especially for eigenvalue problems
    void fill_in_contribution_to_jacobian_and_mass_matrix(
      Vector<double>& residuals,
      DenseMatrix<double>& jacobian,
      DenseMatrix<double>& mass_matrix)
    {
      // Call the generic routine with the flag set to 2
      fill_in_generic_residual_contribution(
        residuals, jacobian, GeneralisedElement::Dummy_matrix, 2);
    }

    /// Overload the output function
    void output(std::ostream& outfile)
    {
      FiniteElement::output(outfile);
    }

    /// Output function: x,y,[z],u,v,[w],p in tecplot format
    void output(std::ostream& outfile, const unsigned& nplot)
    {
      FiniteElement::output(outfile, nplot);
    }

    /// local velocities
    double u(const unsigned& l, const unsigned& i)
    {
      return nodal_value(l, i);
    }

    /// local position
    double x(const unsigned& l, const unsigned& i)
    {
      return nodal_position(l, i);
    }
  };


  ///////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////


  //============================================================================
  /// Function that returns the residuals for the imposed traction Navier_Stokes
  /// equations
  //============================================================================
  template<class ELEMENT>
  void PolarNavierStokesTractionElement<ELEMENT>::
    fill_in_generic_residual_contribution(Vector<double>& residuals,
                                          DenseMatrix<double>& jacobian,
                                          DenseMatrix<double>& mass_matrix,
                                          unsigned flag)
  {
    // Find out how many nodes there are
    unsigned n_node = nnode();

    // Get continuous time from timestepper of first node
    double time = node_pt(0)->time_stepper_pt()->time_pt()->time();

    // Set up memory for the shape and test functions
    Shape psif(n_node), testf(n_node);

    // Set the value of n_intpt
    unsigned n_intpt = integral_pt()->nweight();

    // Get Alpha
    const double Alpha = alpha();

    // Storage for external pressure
    double pext = 0.0;

    // Get boundary multiplier
    // This is necessary because the sign of the traction is
    // dependent on the boundary
    const int multiplier = boundary();

    // Get the homotopy parameter (if necessary)
    const double eta = get_eta();

    // Integers to store local equation numbers
    int local_eqn = 0, local_unknown = 0, pext_local_eqn = 0,
        pext_local_unknown = 0;

    ////////////////////////////////////////NEW//////////////////////////////////////////

    // Get local equation number of external pressure
    // Note that if we have not passed an external pressure pointer to this
    // element (and at the same time added it to the element's external data)
    // than this will be -1 to indicate that it is not a degree of freedom here
    if (Pext_data_pt == 0)
    {
      pext_local_eqn = -1;
    }
    else
    {
      // If at a non-zero degree of freedom add in the entry
      pext_local_eqn = external_local_eqn(External_data_number_of_Pext, 0);

      // Get external pressure
      pext = Pext_data_pt->value(0);
    }

    // The local unkown number of pext will be the same
    pext_local_unknown = pext_local_eqn;

    ////////////////////////////////////////NEW//////////////////////////////////////////

    // Loop over the integration points
    for (unsigned ipt = 0; ipt < n_intpt; ipt++)
    {
      // Get the integral weight
      double w = integral_pt()->weight(ipt);

      // Find the shape and test functions and return the Jacobian
      // of the mapping
      double J = shape_and_test_at_knot(ipt, psif, testf);

      // Premultiply the weights and the Jacobian
      double W = w * J;

      // Need to find position to feed into Traction function
      Vector<double> interpolated_x(Dim);
      Vector<double> interpolated_u(Dim);

      // Initialise to zero
      for (unsigned i = 0; i < Dim; i++)
      {
        interpolated_x[i] = 0.0;
        interpolated_u[i] = 0.0;
      }

      // Calculate velocities and derivatives:
      // Loop over nodes
      for (unsigned l = 0; l < n_node; l++)
      {
        // Loop over directions
        for (unsigned i = 0; i < Dim; i++)
        {
          // Get the nodal value
          interpolated_u[i] += this->nodal_value(l, i) * psif[l];
          interpolated_x[i] += this->nodal_position(l, i) * psif[l];
        }
      }

      // Get the user-defined traction terms
      Vector<double> traction(Dim);
      get_traction(time, interpolated_x, traction);

      // Now add to the appropriate equations

      // Loop over the test functions
      for (unsigned l = 0; l < n_node; l++)
      {
        // Only alter u velocity component
        {
          unsigned i = 0;
          local_eqn = u_local_eqn(l, i);
          /*IF it's not a boundary condition*/
          if (local_eqn >= 0)
          {
            // Add the user-defined traction terms
            residuals[local_eqn] -= multiplier * eta * 3.0 *
                                    (interpolated_u[i] / interpolated_x[0]) *
                                    testf[l] * interpolated_x[0] * Alpha * W;

            ////////////////////////////////////////NEW//////////////////////////////////////////

            // Plus additional external pressure contribution at inlet
            // This is zero if we haven't passed a Pext_data_pt to the element
            residuals[local_eqn] +=
              pext * testf[l] * interpolated_x[0] * Alpha * W;

            ////////////////////////////////////////NEW//////////////////////////////////////////

            // CALCULATE THE JACOBIAN
            if (flag)
            {
              // Loop over the velocity shape functions again
              for (unsigned l2 = 0; l2 < n_node; l2++)
              {
                // We only have an i2=0 contribution
                unsigned i2 = 0;
                {
                  // If at a non-zero degree of freedom add in the entry
                  local_unknown = u_local_eqn(l2, i2);
                  if (local_unknown >= 0)
                  {
                    // Add contribution to Elemental Matrix
                    jacobian(local_eqn, local_unknown) -=
                      multiplier * eta * 3.0 * (psif[l2] / interpolated_x[0]) *
                      testf[l] * interpolated_x[0] * Alpha * W;

                  } // End of (Jacobian's) if not boundary condition statement
                } // End of i2=0 section
              } // End of l2 loop

              ////////////////////////////////////////NEW//////////////////////////////////////////
              // Add pext's contribution to these residuals
              // This only needs to be done once hence why it is outside the l2
              // loop
              if (pext_local_unknown >= 0)
              {
                // Add contribution to Elemental Matrix
                jacobian(local_eqn, pext_local_unknown) +=
                  testf[l] * interpolated_x[0] * Alpha * W;
              }
              ////////////////////////////////////////NEW//////////////////////////////////////////

            } /*End of Jacobian calculation*/

          } // end of if not boundary condition statement
        } // End of i=0 section

      } // End of loop over shape functions

      ////////////////////////////////////////NEW//////////////////////////////////////////

      /// The additional residual for the mass flux
      /// (ie. the extra equation for pext)
      /// This is an integral equation along the whole boundary
      /// It lies outside the loop over shape functions above
      {
        /*IF it's not a boundary condition*/
        if (pext_local_eqn >= 0)
        {
          // Add the user-defined traction terms
          residuals[pext_local_eqn] +=
            interpolated_u[0] * interpolated_x[0] * Alpha * W;

          // No longer necessary due to my FluxCosntraint element
          // Now take off a fraction of the desired mass flux
          // Divided by number of elements and number of int points in each
          // HACK
          // residuals[pext_local_eqn] -= (1.0/(30.*3.));
          // HACK

          // CALCULATE THE JACOBIAN
          if (flag)
          {
            // Loop over the velocity shape functions again
            for (unsigned l2 = 0; l2 < n_node; l2++)
            {
              // We only have an i2=0 contribution
              unsigned i2 = 0;
              {
                // If at a non-zero degree of freedom add in the entry
                local_unknown = u_local_eqn(l2, i2);
                if (local_unknown >= 0)
                {
                  // Add contribution to Elemental Matrix
                  jacobian(pext_local_eqn, local_unknown) +=
                    psif[l2] * interpolated_x[0] * Alpha * W;

                } // End of (Jacobian's) if not boundary condition statement
              } // End of i2=0 section

            } // End of l2 loop
          } /*End of Jacobian calculation*/

        } // end of if not boundary condition statement
      } // End of additional residual for the mass flux

      ////////////////////////////////////////NEW//////////////////////////////////////////

    } // End of loop over integration points

  } // End of fill_in_generic_residual_contribution

} // End of namespace oomph

#endif
