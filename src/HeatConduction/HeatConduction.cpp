#include "HeatConduction.H"

#if BL_SPACEDIM == 2

HeatConduction::HeatConduction() :
  GeneralAMRIntegrator(), 
  mybc(geom)
{
  RegisterNewFab(Temp,     mybc, number_of_components, number_of_ghost_cells, "Temp");
  RegisterNewFab(Temp_old, mybc, number_of_components, number_of_ghost_cells, "Temp old");
}

void
HeatConduction::Advance (int lev, Real /*time*/, Real dt)
{
  std::swap(*Temp[lev], *Temp_old[lev]);

  const Real* dx = geom[lev].CellSize();

  for ( amrex::MFIter mfi(*Temp[lev],true); mfi.isValid(); ++mfi )
    {
      const amrex::Box& bx = mfi.tilebox();

      amrex::BaseFab<Real> &Temp_old_box = (*Temp_old[lev])[mfi];
      amrex::BaseFab<Real> &Temp_box = (*Temp[lev])[mfi];

      for (int i = bx.loVect()[0]; i<=bx.hiVect()[0]; i++)
	for (int j = bx.loVect()[1]; j<=bx.hiVect()[1]; j++)
	  {
	    Temp_box(amrex::IntVect(i,j))
	      = Temp_old_box(amrex::IntVect(i,j))
	      + dt * ((Temp_old_box(amrex::IntVect(i+1,j)) + Temp_old_box(amrex::IntVect(i-1,j)) - 2*Temp_old_box(amrex::IntVect(i,j))) / dx[0] / dx[0] +
		      (Temp_old_box(amrex::IntVect(i,j+1)) + Temp_old_box(amrex::IntVect(i,j-1)) - 2*Temp_old_box(amrex::IntVect(i,j))) / dx[1] / dx[1]);
	  }
    }
}



void
HeatConduction::Initialize (int lev)
{
  //const amrex::Real width = geom[lev].ProbHi()[0] - geom[lev].ProbHi()[1];
  for (amrex::MFIter mfi(*Temp[lev],true); mfi.isValid(); ++mfi)
    {
      const amrex::Box& box = mfi.tilebox();
      amrex::BaseFab<Real> &Temp_box = (*Temp[lev])[mfi];
      amrex::BaseFab<Real> &Temp_old_box = (*Temp_old[lev])[mfi];
      for (int i = box.loVect()[0]-number_of_ghost_cells; i<=box.hiVect()[0]+number_of_ghost_cells; i++) 
	for (int j = box.loVect()[1]-number_of_ghost_cells; j<=box.hiVect()[1]+number_of_ghost_cells; j++)
	  {
	    Temp_box(amrex::IntVect(i,j),0) = 0; 
	    Temp_old_box(amrex::IntVect(i,j),0) = Temp_box(amrex::IntVect(i,j),0);
	  }
    }
}


void
HeatConduction::TagCellsForRefinement (int lev, amrex::TagBoxArray& tags, amrex::Real /*time*/, int /*ngrow*/)
{

  const Real* dx      = geom[lev].CellSize();

  amrex::Array<int>  itags;
 	
  for (amrex::MFIter mfi(*Temp[lev],true); mfi.isValid(); ++mfi)
    {
      const amrex::Box&  bx  = mfi.tilebox();
      amrex::TagBox&     tag  = tags[mfi];
 	    
      amrex::BaseFab<Real> &Temp_old_box = (*Temp_old[lev])[mfi];

      for (int i = bx.loVect()[0]; i<=bx.hiVect()[0]; i++)
	for (int j = bx.loVect()[1]; j<=bx.hiVect()[1]; j++)
	  {
	    amrex::Real grad1 = (Temp_old_box(amrex::IntVect(i+1,j)) - Temp_old_box(amrex::IntVect(i-1,j)))/(2*dx[0]);
	    amrex::Real grad2 = (Temp_old_box(amrex::IntVect(i,j+1)) - Temp_old_box(amrex::IntVect(i,j-1)))/(2*dx[1]);

	    if ((grad1*grad1 + grad2*grad2)*dx[0]*dx[1] > 0.01) tag(amrex::IntVect(i,j)) = amrex::TagBox::SET;

	  }

    }

}

#endif
