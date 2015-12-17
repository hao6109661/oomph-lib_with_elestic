//LIC// ====================================================================
//LIC// This file forms part of oomph-lib, the object-oriented, 
//LIC// multi-physics finite-element library, available 
//LIC// at http://www.oomph-lib.org.
//LIC// 
//LIC//           Version 0.90. August 3, 2009.
//LIC// 
//LIC// Copyright (C) 2006-2009 Matthias Heil and Andrew Hazel
//LIC// 
//LIC// This library is free software; you can redistribute it and/or
//LIC// modify it under the terms of the GNU Lesser General Public
//LIC// License as published by the Free Software Foundation; either
//LIC// version 2.1 of the License, or (at your option) any later version.
//LIC// 
//LIC// This library is distributed in the hope that it will be useful,
//LIC// but WITHOUT ANY WARRANTY; without even the implied warranty of
//LIC// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//LIC// Lesser General Public License for more details.
//LIC// 
//LIC// You should have received a copy of the GNU Lesser General Public
//LIC// License along with this library; if not, write to the Free Software
//LIC// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
//LIC// 02110-1301  USA.
//LIC// 
//LIC// The authors may be contacted at oomph-lib@maths.man.ac.uk.
//LIC// 
//LIC//====================================================================
//Header file for TFoepplvonKarman elements
#ifndef OOMPH_TFOEPPLVONKARMAN_DISPLACEMENT_ELEMENTS_HEADER
#define OOMPH_TFOEPPLVONKARMAN_DISPLACEMENT_ELEMENTS_HEADER


// Config header generated by autoconfig
#ifdef HAVE_CONFIG_H
  #include <oomph-lib-config.h>
#endif


//OOMPH-LIB headers
#include "../generic/nodes.h"
#include "../generic/oomph_utilities.h"
#include "../generic/Telements.h"
#include "../generic/error_estimator.h"

#include "foeppl_von_karman_displacement_elements.h"

namespace oomph
{

/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
// TFoepplvonKarmanDisplacementElement
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

 
// -----------------------------------------------------------------------
// THE TRIANGLE ELEMENT
// -----------------------------------------------------------------------


//======================================================================
/// TFoepplvonKarmanDisplacementElement<NNODE_1D> elements are isoparametric
/// triangular 2-dimensional Foeppl von Karman elements with NNODE_1D
/// nodal points along each element edge. Inherits from TElement and
/// FoepplvonKarmanDisplacementEquations
//======================================================================
template <unsigned NNODE_1D>
class TFoepplvonKarmanDisplacementElement : public virtual TElement<2,NNODE_1D>,
                                      public virtual FoepplvonKarmanDisplacementEquations,
                                      public virtual ElementWithZ2ErrorEstimator
{
  
public:
  
  ///\short  Constructor: Call constructors for TElement and
  /// Foeppl von Karman equations
  TFoepplvonKarmanDisplacementElement() : TElement<2,NNODE_1D>(), 
                                    FoepplvonKarmanDisplacementEquations()
  { }
  
  
  /// Broken copy constructor
  TFoepplvonKarmanDisplacementElement(const TFoepplvonKarmanDisplacementElement<NNODE_1D>& dummy)
  {
    BrokenCopy::broken_copy("TFoepplvonKarmanDisplacementElement");
  }
  
  /// Broken assignment operator
  void operator=(const TFoepplvonKarmanDisplacementElement<NNODE_1D>&)
  {
    BrokenCopy::broken_assign("TFoepplvonKarmanDisplacementElement");
  }
  
  /// \short Access function for Nvalue: # of `values' (pinned or
  /// dofs) at node n (always returns the same value at every node, 4)
  inline unsigned required_nvalue(const unsigned &n) const
  {return Initial_Nvalue;}
  
  /// \short The number of dof types that degrees of freedom in this
  /// element are sub-divided into
  unsigned ndof_types() const
  {
    // NOTE: this assumes "clamped" bcs
    // [0]: laplacian w interior
    // [1]: laplacian w boundary
    // [2]: W
    // [3]: Ux
    // [4]: Uy
    return 5;
  }
  
  /// \short Create a list of pairs for all unknowns in this element,
  /// so that the first entry in each pair contains the global
  /// equation number of the unknown, while the second one contains
  /// the number of the dof type that this unknown is associated with.
  /// (Function can obviously only be called if the equation numbering
  /// scheme has been set up.)
  /// Dof_types
  /// 0,1: Laplacian;
  /// 2: Bending w
  /// 3: Displacements Ux and Uy
  /// The indexing of the dofs in the element is like below
  /// [0]: w
  /// [1]: laplacian w
  /// [2]: U_x
  /// [3]: U_y
  void get_dof_numbers_for_unknowns(
  std::list<std::pair<unsigned long,unsigned> >& dof_lookup_list) const
  {
    // number of nodes
    const unsigned n_node = this->nnode();
    
    // temporary pair (used to store dof lookup prior to being added to list)
    std::pair<unsigned,unsigned> dof_lookup;
    
    // loop over the nodes
    for (unsigned n = 0; n < n_node; n++)
      {        
        // Zeroth nodal value: displacement
        //---------------------------------
        unsigned v=0;
        
        // determine local eqn number
        int local_eqn_number = this->nodal_local_eqn(n, v);
        
        // ignore pinned values
        if (local_eqn_number >= 0)
          {
            // store dof lookup in temporary pair: Global equation
            // number is the first entry in pair
            dof_lookup.first = this->eqn_number(local_eqn_number);
            
            // set dof type numbers: Dof type is the second entry in pair
            dof_lookup.second = 2;
            
            // add to list
            dof_lookup_list.push_front(dof_lookup);
          }
        
        // First nodal value: Laplacian
        //-----------------------------
        v=1;
        
        // determine local eqn number
        local_eqn_number = this->nodal_local_eqn(n, v);
        
        // ignore pinned values
        if (local_eqn_number >= 0)
          {
            // store dof lookup in temporary pair: Global equation
            // number is the first entry in pair
            dof_lookup.first = this->eqn_number(local_eqn_number);
            
            // Is it a boundary node? If so: It's dof type 1
            if (node_pt(n)->is_on_boundary(0) || node_pt(n)->is_on_boundary(1))
              {
                dof_lookup.second = 1;
              }
            // otherwise it's in the interior: It's dof type 0
            else
              {
                dof_lookup.second = 0;
              }
            
            // add to list
            dof_lookup_list.push_front(dof_lookup);
          }
        
        // Second nodal value: U_x
        //---------------------------------
        v=2;
        
        // determine local eqn number
        local_eqn_number = this->nodal_local_eqn(n, v);
        
        // ignore pinned values
        if (local_eqn_number >= 0)
          {
            // store dof lookup in temporary pair: Global equation
            // number is the first entry in pair
            dof_lookup.first = this->eqn_number(local_eqn_number);
            
            // set dof type numbers: Dof type is the second entry in pair
            dof_lookup.second = 3;
            
            // add to list
            dof_lookup_list.push_front(dof_lookup);
          }
        
        // Third nodal value: U_y
        //---------------------------------
        v=3;
        
        // determine local eqn number
        local_eqn_number = this->nodal_local_eqn(n, v);
        
        // ignore pinned values
        if (local_eqn_number >= 0)
          {
            // store dof lookup in temporary pair: Global equation
            // number is the first entry in pair
            dof_lookup.first = this->eqn_number(local_eqn_number);
            
            // set dof type numbers: Dof type is the second entry in pair
            dof_lookup.second = 4;
            
            // add to list
            dof_lookup_list.push_front(dof_lookup);
          }
        
      } // for (n < n_node) 
    
  }
  
  /// \short Output function:
  ///  x,y,w
  void output(std::ostream &outfile)
  {
    FoepplvonKarmanDisplacementEquations::output(outfile);
  }
  
  ///  \short Output function:
  ///   x,y,w at n_plot^2 plot points
  void output(std::ostream &outfile, const unsigned &n_plot)
  {
    FoepplvonKarmanDisplacementEquations::output(outfile,n_plot);
  }
  
  
  /// \short C-style output function:
  ///  x,y,w
  void output(FILE* file_pt)
  {
    FoepplvonKarmanDisplacementEquations::output(file_pt);
  }
  
  
  ///  \short C-style output function:
  ///   x,y,w at n_plot^2 plot points
  void output(FILE* file_pt, const unsigned &n_plot)
  {
    FoepplvonKarmanDisplacementEquations::output(file_pt,n_plot);
  }
  
  
  /// \short Output function for an exact solution:
  ///  x,y,w_exact
  void output_fct(std::ostream &outfile, const unsigned &n_plot,
                  FiniteElement::SteadyExactSolutionFctPt exact_soln_pt)
  {
    FoepplvonKarmanDisplacementEquations::output_fct(outfile,n_plot,exact_soln_pt);
  }
  
  
  /// \short Output function for a time-dependent exact solution.
  ///  x,y,w_exact (calls the steady version)
  void output_fct(std::ostream &outfile, const unsigned &n_plot,
                  const double& time,
                  FiniteElement::UnsteadyExactSolutionFctPt exact_soln_pt)
  {
    FoepplvonKarmanDisplacementEquations::output_fct(outfile,n_plot,time,exact_soln_pt);
  }
  
protected:
  
  /// Shape, test functions & derivs. w.r.t. to global coords. Return Jacobian.
  inline double dshape_and_dtest_eulerian_fvk(const Vector<double> &s,
                                              Shape &psi,
                                              DShape &dpsidx,
                                              Shape &test,
                                              DShape &dtestdx) const;
  
  
  /// Shape, test functions & derivs. w.r.t. to global coords. Return Jacobian.
  inline double dshape_and_dtest_eulerian_at_knot_fvk(const unsigned &ipt,
                                                      Shape &psi,
                                                      DShape &dpsidx,
                                                      Shape &test,
                                                      DShape &dtestdx) const;
  
  /// \short Order of recovery shape functions for Z2 error estimation:
  /// Same order as shape functions.
  unsigned nrecovery_order() {return (NNODE_1D-1);}
  
  /// Number of 'flux' terms for Z2 error estimation
  unsigned num_Z2_flux_terms() {return 2;}//The dimension
    
  /// Get 'flux' for Z2 error recovery:  Standard flux.from FvK equations
  void get_Z2_flux(const Vector<double>& s, Vector<double>& flux)
  {this->get_gradient_of_deflection(s,flux);}
  
  /// \short Number of vertex nodes in the element
  unsigned nvertex_node() const
  {return TElement<2,NNODE_1D>::nvertex_node();}
  
  /// \short Pointer to the j-th vertex node in the element
  Node* vertex_node_pt(const unsigned& j) const
  {return TElement<2,NNODE_1D>::vertex_node_pt(j);}
  
private:
  
  /// Static unsigned that holds the (same) number of variables at every node
  static const unsigned Initial_Nvalue;
  
  
};
  
  
  
  
//Inline functions:
  
  
//======================================================================
/// Define the shape functions and test functions and derivatives
/// w.r.t. global coordinates and return Jacobian of mapping.
///
/// Galerkin: Test functions = shape functions
//======================================================================
template<unsigned NNODE_1D>
double TFoepplvonKarmanDisplacementElement<NNODE_1D>::dshape_and_dtest_eulerian_fvk(
 const Vector<double> &s,
 Shape &psi,
 DShape &dpsidx,
 Shape &test,
 DShape &dtestdx) const
{
  unsigned n_node = this->nnode();
  
  //Call the geometrical shape functions and derivatives
  double J = this->dshape_eulerian(s,psi,dpsidx);
  
  //Loop over the test functions and derivatives and set them equal to the
  //shape functions
  for(unsigned i=0;i<n_node;i++)
    {
      test[i] = psi[i];
      dtestdx(i,0) = dpsidx(i,0);
      dtestdx(i,1) = dpsidx(i,1);
    }
  
  //Return the jacobian
  return J;
}
  
  
  
//======================================================================
/// Define the shape functions and test functions and derivatives
/// w.r.t. global coordinates and return Jacobian of mapping.
///
/// Galerkin: Test functions = shape functions
//======================================================================
template<unsigned NNODE_1D>
double TFoepplvonKarmanDisplacementElement<NNODE_1D>::
dshape_and_dtest_eulerian_at_knot_fvk(
                                      const unsigned &ipt,
                                      Shape &psi,
                                      DShape &dpsidx,
                                      Shape &test,
                                      DShape &dtestdx) const
{
  
  //Call the geometrical shape functions and derivatives
  double J = this->dshape_eulerian_at_knot(ipt,psi,dpsidx);
  
  //Set the pointers of the test functions
  test = psi;
  dtestdx = dpsidx;
  
  //Return the jacobian
  return J;
  
}
  
  
//=======================================================================
/// Face geometry for the TFoepplvonKarmanDisplacementElement
//  elements:The spatial / dimension of the face elements is one lower
//  than that of the / bulk element but they have the same number of
//  points / along their 1D edges.
//=======================================================================
template<unsigned NNODE_1D>
class FaceGeometry<TFoepplvonKarmanDisplacementElement<NNODE_1D> >:
 public virtual TElement<1,NNODE_1D>
{
  
public:
  
  /// \short Constructor: Call the constructor for the
  /// appropriate lower-dimensional TElement
  FaceGeometry() : TElement<1,NNODE_1D>() {}
  
};

} // namespace oomph

#endif