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
//Header file for multi-domain functions, including the class
//ElementWithExternalElement which stores pointers to external elements

//Include guards to prevent multiple inclusion of the header
#ifndef OOMPH_MULTI_DOMAIN_HEADER
#define OOMPH_MULTI_DOMAIN_HEADER

// Config header generated by autoconfig
#ifdef HAVE_CONFIG_H
#include <oomph-lib-config.h>
#endif

//Oomph-lib headers
#include "geom_objects.h"
#include "problem.h"
#include "shape.h"

#include "mesh.h"
#include "mesh_as_geometric_object.h"
#include "algebraic_elements.h"
#include "macro_element_node_update_element.h"
#include "Qelements.h"
#include "element_with_external_element.h"

namespace oomph
{

//======================================================================
// Namespace for global multi-domain functions
//======================================================================
namespace Multi_domain_functions
 {

  /// \short Dimension of zeta tuples (set by get_dim_helper) -- needed
  /// because we store the scalar coordinates in flat-packed form.
  extern unsigned Dim;

  /// \short Lookup scheme for whether a local element's integration point
  /// has had an external element assigned to it -- essentially boolean.
  /// External_element_located[e][ipt] = {0,1} if external element
  /// for ipt-th integration in local element e {has not, has} been found.
  /// Used locally to ensure that we're not searching for the same
  /// elements over and over again when we go around the spirals.
  extern Vector<Vector<unsigned> > External_element_located;
  
  /// \short Vector of flat-packed zeta coordinates for which the external
  /// element could not be found during current local search. These
  /// will be sent to the next processor in the ring-like parallel search.
  /// The zeta coordinates come in groups of Dim (scalar) coordinates.
  extern Vector<double> Flat_packed_zetas_not_found_locally;

  /// \short Vector of flat-packed zeta coordinates for which the external
  /// element could not be found on another processor and for which
  /// we're currently searching here. Whatever can't be found here,
  /// gets written into Flat_packed_zetas_not_found_locally and then
  /// passed on to the next processor during the ring-like parallel search.
  /// The zeta coordinates come in  groups of Dim (scalar) coordinates.
  extern Vector<double> Received_flat_packed_zetas_to_be_found;

  /// \short Proc_id_plus_one_of_external_element[i] contains the 
  /// processor id (plus one) of the processor
  /// on which the i-th zeta coordinate tuple received from elsewhere 
  /// (in the order in which these are stored in 
  /// Received_flat_packed_zetas_to_be_found) was located; it's zero if
  /// it wasn't found during the current stage of the ring-like parallel
  /// search.
  extern Vector<int> Proc_id_plus_one_of_external_element;

  /// \short Vector to indicate (to another processor) whether a
  /// located element (that will have to represented as an external
  /// halo element on that processor) should be newly created on that 
  /// processor (2), already exists on that processor (1), or
  /// is not on the current processor either (0).
  extern Vector<unsigned> Located_element_status;

  /// \short Vector of flat-packed local coordinates for zeta tuples
  /// that have been located
  extern Vector<double> Flat_packed_located_coordinates;

  /// \short Vector of flat-packed doubles to be communicated with
  /// other processors
  extern Vector<double> Flat_packed_doubles;

  /// \short Counter used when processing vector of flat-packed 
  /// doubles -- this is really "private" data, declared here
  /// to avoid having to pass it (and the associated array)
  /// between the various helper functions
  extern unsigned Counter_for_flat_packed_doubles;

  /// \short Vector of flat-packed unsigneds to be communicated with
  /// other processors -- this is really "private" data, declared here
  /// to avoid having to pass the array between the various helper 
  /// functions
  extern Vector<unsigned> Flat_packed_unsigneds;

  /// \short Counter used when processing vector of flat-packed 
  /// unsigneds -- this is really "private" data, declared here
  /// to avoid having to pass it (and the associated array)
  /// between the various helper functions
  extern unsigned Counter_for_flat_packed_unsigneds;

  /// \short Enumerators for element status in location procedure
  enum{ New, Exists, Not_found};

  // Default parameters for the binning method
  //------------------------------------------

  /// \short Bool to tell the MeshAsGeomObject whether to calculate
  /// the extreme coordinates of the bin structure
  extern bool Compute_extreme_bin_coordinates;

  /// \short Number of bins in the first dimension in binning method in
  /// setup_multi_domain_interaction().
  extern unsigned Nx_bin;

  /// \short Number of bins in the second dimension in binning method in
  /// setup_multi_domain_interaction().
  extern unsigned Ny_bin;

  /// \short Number of bins in the third dimension in binning method in
  /// setup_multi_domain_interaction().
  extern unsigned Nz_bin;

  /// Number of spirals to be searched in one go
  extern unsigned N_spiral_chunk;

  /// \short (Measure of) the number of sampling points within the elements 
  /// when populating the bin
  extern unsigned Nsample_points;

  /// \short Minimum and maximum coordinates for
  /// each dimension of the bin structure used in
  /// MeshAsGeomObject::locate_zeta(...). 
  /// These can be set by user if they know their mesh's extreme coordinates
  /// (or the MeshAsGeomObject calculates these values by default based
  /// upon the mesh itself; see MeshAsGeomObject::get_max_and_min_coords(...))
  /// They default to "incorrect" values initially.

  /// \short Minimum coordinate in first dimension
  extern double X_min;

  /// \short Maximum coordinate in first dimension
  extern double X_max;

  /// \short Minimum coordinate in second dimension
  extern double Y_min;

  /// \short Maximum coordinate in second dimension
  extern double Y_max;

  /// \short Minimum coordinate in third dimension
  extern double Z_min;

  /// \short Maximum coordinate in third dimension
  extern double Z_max;

  /// \short Percentage offset to add to each extreme of the bin structure.
  /// Default value of 0.05.
  extern double Percentage_offset;

  /// \short Boolean to indicate when to use the bulk element as the
  /// external element.  Defaults to false, you must have set up FaceElements
  /// properly first in order for it to work
  extern bool Use_bulk_element_as_external;

  /// \short Boolean to indicate whether to doc timings or not.
  extern bool Doc_timings;

  /// \short Boolean to indicate whether to document basic info (to screen)
  ///        during setup_multi_domain_interaction() routines
  extern bool Doc_stats;

  /// \short Boolean to indicate whether to document further info (to screen)
  ///        during setup_multi_domain_interaction() routines
  extern bool Doc_full_stats;

#ifdef OOMPH_HAS_MPI
  /// \short Boolean to indicate when to check for duplicate data
  ///        between the external halo storage schemes  
  extern bool Check_for_duplicates;
#endif

  // Functions for multi-domain method

  /// \short Set up the two-way multi-domain interactions for the 
  /// problem pointed to by \c problem_pt.
  /// Use this for cases where first_mesh_pt and second_mesh_pt
  /// occupy the same physical space and are populated by
  /// ELEMENT_0 and ELEMENT_1 respectively, and are combined to solve
  /// a single problem. The elements in two meshes interact both ways
  /// the elements in each mesh act as "external elements" for the 
  /// elements in the "other" mesh. The interaction indices allow the 
  /// specification of which interaction we're setting up in the two 
  /// meshes. They default to zero, which is appropriate if there's 
  /// only a single interaction.
  template<class ELEMENT_0,class ELEMENT_1>
   void setup_multi_domain_interactions(Problem* problem_pt, 
                                        Mesh* const &first_mesh_pt,
                                        Mesh* const &second_mesh_pt,
                                        const unsigned& first_interaction=0,
                                        const unsigned& second_interaction=0);

  /// \short Function to set up the one-way multi-domain interaction for 
  /// problems where the meshes pointed to by \c mesh_pt and \c external_mesh_pt
  /// occupy the same physical space, and the elements in \c external_mesh_pt
  /// act as "external elements" for the \c ElementWithExternalElements
  /// in \c mesh_pt (but not vice versa):
  /// - \c mesh_pt points to the mesh of ElemenWithExternalElements for which
  ///   the interaction is set up. 
  /// - \c external_mesh_pt points to the mesh that contains the elements
  ///   of type EXT_ELEMENT that act as "external elements" for the
  ///   \c ElementWithExternalElements in \ mesh_pt.
  /// - The interaction_index parameter defaults to zero and must be otherwise
  ///   set by the user if there is more than one mesh that provides sources
  ///   for the Mesh pointed to by mesh_pt.
  template<class EXT_ELEMENT>
   void setup_multi_domain_interaction(Problem* problem_pt,
                                       Mesh* const &mesh_pt,
                                       Mesh* const &external_mesh_pt,
                                       const unsigned& interaction_index=0);

  /// \short Function to set up the one-way multi-domain interaction for 
  /// FSI-like problems. 
  /// - \c mesh_pt points to the mesh of \c ElemenWithExternalElements for which
  ///   the interaction is set up. In an FSI example, this mesh would contain
  ///   the \c FSIWallElements (either beam/shell elements or the
  ///   \c FSISolidTractionElements that apply the traction to 
  ///   a "bulk" solid mesh that is loaded by the fluid.)
  /// - \c external_mesh_pt points to the mesh that contains the elements
  ///   of type EXT_ELEMENT that provide the "source" for the
  ///   \c ElementWithExternalElements. In an FSI example, this 
  ///   mesh would contain the "bulk" fluid elements.
  /// - \c external_face_mesh_pt points to the mesh of \c FaceElements
  ///   attached to the \c external_mesh_pt. The mesh pointed to by
  ///   \c external_face_mesh_pt has the same dimension as \c mesh_pt.
  ///   The elements contained in \c external_face_mesh_pt are of type 
  ///   FACE_ELEMENT_GEOM_OBJECT. In an FSI example, these elements
  ///   are usually the \c FaceElementAsGeomObjects (templated by the
  ///   type of the "bulk" fluid elements to which they are attached)
  ///   that define the FSI boundary of the fluid domain.
  /// - The interaction_index parameter defaults to zero and must otherwise be
  ///   set by the user if there is more than one mesh that provides "external
  ///   elements" for the Mesh pointed to by mesh_pt (e.g. in the case
  ///   when a beam or shell structure is loaded by fluid from both sides.)
  template<class EXT_ELEMENT, class FACE_ELEMENT_GEOM_OBJECT>
   void setup_multi_domain_interaction(Problem* problem_pt,
                                       Mesh* const &mesh_pt,
                                       Mesh* const &external_mesh_pt,
                                       Mesh* const &external_face_mesh_pt,
                                       const unsigned& interaction_index=0);
  
  /// \short Auxiliary function which is called from the two preceding
  /// functions 
  template<class EXT_ELEMENT, class GEOM_OBJECT>
   void aux_setup_multi_domain_interaction(Problem* problem_pt,
                                           Mesh* const &mesh_pt,
                                           Mesh* const &external_mesh_pt,
                                           const unsigned& interaction_index,
                                           Mesh* const &external_face_mesh_pt=0);
  
#ifdef OOMPH_HAS_MPI
  /// \short A helper function to remove duplicate data that 
  /// are created due to coincident nodes between external halo elements 
  /// on different processors
  void remove_duplicate_data(Problem* problem_pt, Mesh* const &mesh_pt);
#endif
  
  /// \short Helper function to locate "local" zeta coordinates
   void locate_zeta_for_local_coordinates
   (Mesh* const &mesh_pt, Mesh* const &external_mesh_pt,
    MeshAsGeomObject* &mesh_geom_obj_pt,
    const unsigned& interaction_index);

#ifdef OOMPH_HAS_MPI
  /// \short Helper function to send any "missing" zeta coordinates to
  /// the next process and receive any coordinates from previous process
  void send_and_receive_missing_zetas(Problem* problem_pt);

  /// \short Helper function to locate these "missing" zeta coordinates
  void locate_zeta_for_missing_coordinates(
   int& iproc, Mesh* const &external_mesh_pt,Problem* problem_pt,
    MeshAsGeomObject* &mesh_geom_obj_pt);

  /// \short Helper function to send back any located information
  void send_and_receive_located_info(int& iproc, Mesh* const &external_mesh_pt,
                                     Problem* problem_pt);

  /// \short Helper function to create external (halo) elements on the loop 
  /// process based on the info received in send_and_received_located_info
  template<class EXT_ELEMENT>
   void create_external_halo_elements(int& iproc, Mesh* const &mesh_pt,
                                      Mesh* const &external_mesh_pt,
                                      Problem* problem_pt,
                                      const unsigned& interaction_index);

  // Helper functions for external haloed node identification

  /// \short Helper function to add external haloed nodes, inc. masters
  void add_external_haloed_node_to_storage(int& iproc, Node* nod_pt,
                                           Problem* problem_pt,
                                           Mesh* const &external_mesh_pt,
                                           int& n_cont_inter_values);


  /// \short Helper function to add external haloed node that is not a master
  void add_external_haloed_node_helper(int& iproc, Node* nod_pt,
                                       Problem* problem_pt,
                                       Mesh* const &external_mesh_pt,
                                       int& n_cont_inter_values);

  /// \short Helper function to add external haloed node that is a master
  void add_external_haloed_master_node_helper(int& iproc,Node* master_nod_pt,
                                              Problem* problem_pt,
                                              Mesh* const &external_mesh_pt,
                                              int& n_cont_inter_values);

  /// \short Helper function to get the required nodal information from an
  /// external haloed node so that a fully-functional external halo
  /// node (and therefore element) can be created on the receiving process
  void get_required_nodal_information_helper(int& iproc, Node* nod_pt,
                                             Problem* problem_pt,
                                             Mesh* const &external_mesh_pt,
                                             int& n_cont_inter_values);

  /// \short Helper function to get the required master nodal information from
  /// an external haloed master node so that a fully-functional external halo
  /// master node (and possible element) can be created on the receiving proc
  void get_required_master_nodal_information_helper
   (int& iproc, Node* master_nod_pt, Problem* problem_pt,
    Mesh* const &external_mesh_pt, int& n_cont_inter_values);

  // Helper functions for external halo node identification

  /// \short Helper function to add external halo nodes, including any masters,
  /// based on information received from the haloed process
  template<class EXT_ELEMENT>
   void add_external_halo_node_to_storage(Node* &new_nod_pt,
                                          Mesh* const &external_mesh_pt,
                                          unsigned& loc_p,
                                          unsigned& node_index,
                                          FiniteElement* const &new_el_pt,
                                          int& n_cont_inter_values,
                                          Problem* problem_pt);

  /// \short Helper function to add external halo node that is not a master
   void add_external_halo_node_helper(Node* &new_nod_pt,
                                      Mesh* const &external_mesh_pt,
                                      unsigned& loc_p,
                                      unsigned& node_index,
                                      FiniteElement* const &new_el_pt,
                                      int& n_cont_inter_values,
                                      Problem* problem_pt);

  /// \short Helper function to add external halo node that is a master
  template<class EXT_ELEMENT>
   void add_external_halo_master_node_helper(Node* &new_master_nod_pt,
                                             Node* &new_nod_pt,
                                             Mesh* const &external_mesh_pt,
                                             unsigned& loc_p,
                                             int& n_cont_inter_values,
                                             Problem* problem_pt);


  /// \short Helper function which constructs a new external halo node 
  /// (on an element) with the information sent from the haloed process
   void construct_new_external_halo_node_helper(Node* &new_nod_pt,
                                                unsigned& loc_p,
                                                unsigned& node_index,
                                                FiniteElement* const 
                                                &new_el_pt,
                                                Mesh* const &external_mesh_pt,
                                                Problem* problem_pt);

  /// \short Helper function which constructs a new external halo master node
  /// with the information sent from the haloed process
  template<class EXT_ELEMENT>
   void construct_new_external_halo_master_node_helper
   (Node* &new_master_nod_pt,Node* &nod_pt,unsigned& loc_p,
    Mesh* const &external_mesh_pt,Problem* problem_pt);

#endif

  /// \short Helper function that computes the dimension of the elements within
  /// each of the specified meshes (and checks they are the same)
  /// Stores result in Dim.
  void get_dim_helper(Problem* problem_pt, Mesh* const &mesh_pt, 
                      Mesh* const &external_mesh_pt);

  /// \short Helper function that clears all the intermediate information used
  /// during the external storage creation at the end of the procedure
  void clean_up();

 }


}

#endif




 
