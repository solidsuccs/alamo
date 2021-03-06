#ifndef MODEL_SOLID_PLASTIC_CRYSTALPLASTICDEGRADABLE_H_
#define MODEL_SOLID_PLASTIC_CRYSTALPLASTICDEGRADABLE_H_

#include "AMReX.H"
#include <AMReX_REAL.H>
#include <eigen3/Eigen/Core>
#include "Affine.H"
#include "Set/Set.H"
#include "Model/Solid/Affine/CubicDegradable.H"

namespace Model
{
namespace Solid
{
namespace Affine
{
class CrystalPlasticDegradable : public CubicDegradable
{
public:
    CrystalPlasticDegradable(){};
    CrystalPlasticDegradable(Solid<Set::Sym::MajorMinor> base) : CubicDegradable(base) {};
    CrystalPlasticDegradable(Set::Scalar a_C11, Set::Scalar a_C12, Set::Scalar a_C44, Set::Scalar a_phi1, Set::Scalar a_Phi, Set::Scalar a_phi2, LatticeType lattice=LatticeType::FCC)
    {
        Eigen::Matrix3d m;
    	m =     Eigen::AngleAxisd(a_phi2, Eigen::Vector3d::UnitX()) *
    		Eigen::AngleAxisd(a_Phi,  Eigen::Vector3d::UnitZ()) *
    	 	Eigen::AngleAxisd(a_phi1, Eigen::Vector3d::UnitX());
        Define(a_C11, a_C12, a_C44, m, lattice);
    }
    CrystalPlasticDegradable(Set::Scalar a_C11, Set::Scalar a_C12, Set::Scalar a_C44, Eigen::Matrix3d m, LatticeType lattice=LatticeType::FCC)
    {
        Define(a_C11, a_C12, a_C44, m, lattice);
    }

    void Define(Set::Scalar a_C11, Set::Scalar a_C12, Set::Scalar a_C44, Eigen::Matrix3d m, LatticeType lattice=LatticeType::FCC)
    {
        amrex::Vector<Set::Scalar> tcrss, exp0, gam0dot;
        if(lattice == LatticeType::FCC)
        {
            tcrss.resize(12); exp0.resize(12); gam0dot.resize(12);
            for (int i=0; i<12; i++)
            {
                tcrss[i] = 1.0e1; // default values.
                exp0[i] = 2.0;
                gam0dot[i] = 0.01;
            }
        }
        else 
            Util::Abort(INFO, "This type of lattice has not been implemented yet");
        Define(a_C11, a_C12, a_C44, m, tcrss, exp0, gam0dot, lattice);
    }
    
    void Define(Set::Scalar a_C11, Set::Scalar a_C12, Set::Scalar a_C44, Eigen::Matrix3d a_m, amrex::Vector<Set::Scalar> &a_tcrss, amrex::Vector<Set::Scalar> &a_exp, amrex::Vector<Set::Scalar> &a_gam0dot, LatticeType lattice=LatticeType::FCC)
    {
        if(lattice == LatticeType::FCC)
        {
            if(a_tcrss.size()!=12 || a_exp.size() !=12 || a_gam0dot.size() != 12) 
                Util::Abort(INFO, "Incorrect size of slip system input information");
            
            int size = 12; 
            Set::Scalar s = AMREX_D_PICK(1.0,1.0,INVSQ2);
            Set::Scalar n = AMREX_D_PICK(1.0, INVSQ2, INVSQ3);
            Set::Matrix m = Set::Matrix::Zero();
            AMREX_D_TERM(
                m(0,0) = a_m(0,0);
                ,
                m(0,1) = a_m(0,1); m(1,0) = a_m(1,0); m(1,1) = a_m(1,1);
                ,
                m(0,2) = a_m(0,2); m(1,2) = a_m(1,2); m(2,0) = a_m(2,0); m(2,1) = a_m(2,1); m(2,2) = a_m(2,2);
            );
            
            slip.resize(size);
            slip[0].n = m*Set::Vector(AMREX_D_DECL(n, n, n));
            slip[1].n = m*Set::Vector(AMREX_D_DECL(n, n, n));
            slip[2].n = m*Set::Vector(AMREX_D_DECL(n, n, n));
            slip[0].s = m*Set::Vector(AMREX_D_DECL(0.0, -s, s));
            slip[1].s = m*Set::Vector(AMREX_D_DECL(s, 0.0, -s));
            slip[2].s = m*Set::Vector(AMREX_D_DECL(-s, s, 0.0));

            slip[3].n = m*Set::Vector(AMREX_D_DECL(-n, -n, n));
            slip[4].n = m*Set::Vector(AMREX_D_DECL(-n, -n, n));
            slip[5].n = m*Set::Vector(AMREX_D_DECL(-n, -n, n));
            slip[3].s = m*Set::Vector(AMREX_D_DECL(0.0, s, s));
            slip[4].s = m*Set::Vector(AMREX_D_DECL(-s, 0.0, -s));
            slip[5].s = m*Set::Vector(AMREX_D_DECL(s, -s, 0.0));

            slip[6].n = m*Set::Vector(AMREX_D_DECL(-n, n, n));
            slip[7].n = m*Set::Vector(AMREX_D_DECL(-n, n, n));
            slip[8].n = m*Set::Vector(AMREX_D_DECL(-n, n, n));
            slip[6].s = m*Set::Vector(AMREX_D_DECL(0.0, -s, s));
            slip[7].s = m*Set::Vector(AMREX_D_DECL(-s, 0.0, -s));
            slip[8].s = m*Set::Vector(AMREX_D_DECL(s, s, 0.0));

            slip[9].n = m*Set::Vector(AMREX_D_DECL(n, -n, n));
            slip[10].n = m*Set::Vector(AMREX_D_DECL(n, -n, n));
            slip[11].n = m*Set::Vector(AMREX_D_DECL(n, -n, n));
            slip[9].s = m*Set::Vector(AMREX_D_DECL(0.0, s, s));
            slip[10].s = m*Set::Vector(AMREX_D_DECL(s, 0.0, -s));
            slip[11].s = m*Set::Vector(AMREX_D_DECL(-s, -s, 0.0));

            for (int i = 0; i < size; i++)
            {
                slip[i].on = true;
                slip[i].tcrss = a_tcrss[i];
                slip[i].tcrss0 = a_tcrss[i];
                slip[i].s_exp = a_exp[i];
                slip[i].gam0dot = a_gam0dot[i];
            }
        }
        else
            Util::Abort(INFO, "Slip system not implemented yet");

        prev = PlasticState::Zero();
        curr = PlasticState::Zero();

        CubicDegradable::Define(a_C11, a_C12, a_C44, a_m);
    }

    void Define(Set::Scalar a_C11, Set::Scalar a_C12, Set::Scalar a_C44, Eigen::Matrix3d m, amrex::Vector<SlipSystem> &a_slip)
    {
        slip = a_slip;

        prev = PlasticState::Zero();
        curr = PlasticState::Zero();
        
        CubicDegradable::Define(a_C11, a_C12, a_C44, m);
    }

    void DegradeYieldSurface(Set::Scalar eta)
    {
        if (std::isnan(eta) || std::isinf(eta)) Util::Abort(INFO, "Nans or Infs in eta. ", eta);
        if (eta < 0.0 || eta > 1.0) Util::Abort(INFO, "Invalid value of eta. ", eta);
        for (int i=0; i<slip.size(); i++)
            slip[i].tcrss = slip[i].tcrss0/eta;
    }

    void DegradeYieldSurface(amrex::Vector<Set::Scalar> &eta)
    {
        if(eta.size() ==0 || eta.size() != slip.size()) Util::Abort(INFO, "Inconsistent size of eta. ", eta.size());
        for (int i=0; i<slip.size(); i++)
            slip[i].tcrss = slip[i].tcrss0/eta[i];
    }

    Set::Scalar YieldSurface()
    {
        Set::Matrix yield = Set::Matrix::Zero(), R = Set::Matrix::Zero();
        for(int i=0; i<slip.size(); i++)
        {
            for (int j = 0; j < AMREX_SPACEDIM; j++)
                for (int k = 0; k < AMREX_SPACEDIM; k++)
                    R(j,k) = slip[i].s(j)*slip[i].n(k);
            yield += slip[i].tcrss*R;
        }
        return yield.norm();
    }

    Set::Scalar OriginalYieldSurface()
    {
        Set::Matrix yield = Set::Matrix::Zero(), R = Set::Matrix::Zero();
        for(int i=0; i<slip.size(); i++)
        {
            for (int j = 0; j < AMREX_SPACEDIM; j++)
                for (int k = 0; k < AMREX_SPACEDIM; k++)
                    R(j,k) = slip[i].s(j)*slip[i].n(k);
            yield += slip[i].tcrss0*R;
        }
        return yield.norm();
    }

    Set::Scalar OriginalPlasticEnergy()
    {
        Set::Matrix yield = Set::Matrix::Zero(), R = Set::Matrix::Zero();
        for(int i=0; i<slip.size(); i++)
        {
            for (int j = 0; j < AMREX_SPACEDIM; j++)
                for (int k = 0; k < AMREX_SPACEDIM; k++)
                    R(j,k) = slip[i].s(j)*slip[i].n(k);
            yield += slip[i].tcrss0*R;
        }
        return yield.norm()*curr.alpha;
    }

    Set::Scalar OriginalPlasticEnergy(Set::Scalar alpha)
    {
        Set::Matrix yield = Set::Matrix::Zero(), R = Set::Matrix::Zero();
        for(int i=0; i<slip.size(); i++)
        {
            for (int j = 0; j < AMREX_SPACEDIM; j++)
                for (int k = 0; k < AMREX_SPACEDIM; k++)
                    R(j,k) = slip[i].s(j)*slip[i].n(k);
            yield += slip[i].tcrss0*R;
        }
        return yield.norm()*alpha;
    }

    Set::Scalar PlasticEnergy()
    {
        Set::Matrix yield = Set::Matrix::Zero(), R = Set::Matrix::Zero();
        for(int i=0; i<slip.size(); i++)
        {
            for (int j = 0; j < AMREX_SPACEDIM; j++)
                for (int k = 0; k < AMREX_SPACEDIM; k++)
                    R(j,k) = slip[i].s(j)*slip[i].n(k);
            yield += slip[i].tcrss*R;
        }
        return yield.norm()*curr.alpha;
    }

    void EvolvePlasticStrain(Set::Matrix sigma, Set::Matrix /*strain*/, Set::Scalar dt)
    {
        curr.epsp = prev.epsp;
        Set::Matrix R = Set::Matrix::Zero();
        for (int i = 0; i < slip.size(); i++)
        {
            for (int j = 0; j < AMREX_SPACEDIM; j++)
                for (int k = 0; k < AMREX_SPACEDIM; k++)
                    R(j,k) = slip[i].s(j)*slip[i].n(k);

            if(slip[i].s.dot(sigma*slip[i].n)/slip[i].tcrss < 1.0) continue;

            if(slip[i].s_exp == 1.0)
                curr.epsp += dt*slip[i].gam0dot*(slip[i].s.dot(sigma*slip[i].n)/slip[i].tcrss)*R;
            else if(slip[i].s_exp == 2.0)
                curr.epsp += dt*slip[i].gam0dot*(slip[i].s.dot(sigma*slip[i].n)/slip[i].tcrss)*(slip[i].s.dot(sigma*slip[i].n)/slip[i].tcrss)*R;
            else if(slip[i].s_exp == 3.0)
                curr.epsp += dt*slip[i].gam0dot*(slip[i].s.dot(sigma*slip[i].n)/slip[i].tcrss)*(slip[i].s.dot(sigma*slip[i].n)/slip[i].tcrss)*(slip[i].s.dot(sigma*slip[i].n)/slip[i].tcrss)*R;
            else
                curr.epsp += dt*slip[i].gam0dot*std::pow((slip[i].s.dot(sigma*slip[i].n)/slip[i].tcrss), slip[i].s_exp)*R;
        }
        curr.alpha = sqrt(2.0/3.0)*curr.epsp.norm();
        CubicDegradable::SetF0(curr.epsp);
    }

    void SetPlasticStrains(PlasticState &a_state)
    {
        prev = a_state;
        curr = a_state;
    }

    PlasticState GetPlasticState()
    {
        return curr;
    }

private: // this is to fix an issue with the intel compiler and constexpr
  constexpr static Set::Scalar INVSQ2 = 0.70710678118654752440084436210484903928483593768847403658833986899536623923105351942519376716382078636750692311545614851246241802792536860632206074854996791570661133296375279637789997525057639103028574;
  constexpr static Set::Scalar INVSQ3 = 0.57735026918962576450914878050195745564760175127012687601860232648397767230293334569371539558574952522520871380513556767665664836499965082627055183736479121617603107730076852735599160670036155830775501;
  //constexpr static Set::Scalar INVSQ2 = 1.0/sqrt(2.0);
  //constexpr static Set::Scalar INVSQ3 = 1.0/sqrt(3.0);
public:
    amrex::Vector<SlipSystem> slip; 
    PlasticState curr, prev;

public:
    static void Parse(CrystalPlasticDegradable & value, IO::ParmParse & pp)
    {
        Set::Scalar C11 = 1.68, C12 = 1.21, C44 = 0.75;
        pp.query("C11",C11);
        pp.query("C12",C12);
        pp.query("C44",C44);

        //if (pp.contains("random"))
        //{
        //    value = CubicCrystalPlastic::Random(C11,C12,C44);
        //    return;
        //}

        Set::Scalar phi1 = 0.0, Phi = 0.0, phi2 = 0.0;
        pp.query("phi1",phi1);
        pp.query("Phi",Phi);
        pp.query("phi2",phi2);

        Eigen::Matrix3d m;
        m = Eigen::AngleAxisd(phi2, Eigen::Vector3d::UnitX()) *
            Eigen::AngleAxisd(Phi,  Eigen::Vector3d::UnitZ()) *
            Eigen::AngleAxisd(phi1, Eigen::Vector3d::UnitX());
        
        std::string lattice;
        LatticeType input_lattice;
        int number_of_slip_systems;
        bool latticeinput = false;

        std::map<std::string, LatticeType> latticemap;
        std::map<LatticeType, int> latticeslipsystemmap;

        amrex::Vector<Set::Scalar> slipplanes, slipdirections;
        amrex::Vector<Set::Vector> input_slipplanes, input_slipdirections;

        latticemap["fcc"] = LatticeType::FCC;
        latticemap["FCC"] = LatticeType::FCC;
        latticemap["bcc"] = LatticeType::BCC;
        latticemap["BCC"] = LatticeType::BCC;
        latticemap["hcp"] = LatticeType::HCP;
        latticemap["HCP"] = LatticeType::HCP;
        
        latticeslipsystemmap[LatticeType::FCC] = 12;
        latticeslipsystemmap[LatticeType::BCC] = 48;
        latticeslipsystemmap[LatticeType::HCP] = 2;
        if(pp.contains("lattice"))
        {
            pp.query("lattice",lattice);
            input_lattice = latticemap[lattice];
            number_of_slip_systems = latticeslipsystemmap[input_lattice];
            latticeinput = true;
        }
        else if (pp.contains("slipplane") && pp.contains("slipdirections"))
        {
            pp.queryarr("slipplane",slipplanes);
            pp.queryarr("slipdirections",slipdirections);
            if(slipplanes.size() <=0 || slipplanes.size()%AMREX_SPACEDIM != 0 || slipdirections.size()%AMREX_SPACEDIM !=0 || slipplanes.size() != slipdirections.size()) Util::Abort(INFO, "Insufficient information on slip system");
            for(int i = 0; i < slipdirections.size()/AMREX_SPACEDIM; i++)
            {
                input_slipdirections.push_back(Set::Vector(AMREX_D_DECL(slipdirections[i],slipdirections[i+1],slipdirections[i+2])));
                input_slipplanes.push_back(Set::Vector(AMREX_D_DECL(slipplanes[i],slipplanes[i+1],slipplanes[i+2])));
            }
            for (int i = 0; i < input_slipdirections.size(); i++)
            {
                if(input_slipdirections[i].norm() ==0 || input_slipplanes[i].norm() == 0) Util::Abort(INFO, "invalid values of slip planes  or directions");
                input_slipdirections[i] = input_slipdirections[i]/input_slipdirections[i].norm();
                input_slipplanes[i] = input_slipplanes[i]/input_slipplanes[i].norm();
            }
            number_of_slip_systems = input_slipdirections.size();
        }
        else
        {
            input_lattice = LatticeType::FCC;
            number_of_slip_systems = latticeslipsystemmap[input_lattice];
            latticeinput = true;
        }

        amrex::Vector<Set::Scalar> input_tcrss, input_exp, input_gam0dot;

        if(pp.contains("tcrss"))
        {
            pp.queryarr("tcrss", input_tcrss);
            if(input_tcrss.size() <= 0 || input_tcrss.size() != number_of_slip_systems) Util::Abort(INFO, "Invalid number of values in tcrss");
        }
        else
        {
            input_tcrss.resize(number_of_slip_systems);
            for (int i = 0; i< number_of_slip_systems; i++) input_tcrss[i] = 1.0e1;
        }
        if(pp.contains("exp"))
        {
            pp.queryarr("exp",input_exp);
            if(input_exp.size() <= 0 || input_exp.size() != number_of_slip_systems) Util::Abort(INFO, "Invalid number of values in exp");
        }
        else
        {
            input_exp.resize(number_of_slip_systems);
            for (int i = 0; i< number_of_slip_systems; i++) input_exp[i] = 2.0;
        }
        if(pp.contains("gam0dot"))
        {
            pp.queryarr("gam0dot",input_gam0dot);
            if(input_gam0dot.size() <= 0 || input_gam0dot.size() != number_of_slip_systems) Util::Abort(INFO, "Invalid number of values in gam0dot");
        }
        else
        {
            input_gam0dot.resize(number_of_slip_systems);
            for (int i = 0; i< number_of_slip_systems; i++) input_gam0dot[i] = 0.01;
        }

        if(latticeinput)
        {
            value.Define(C11,C12,C44,m,input_tcrss,input_exp,input_gam0dot, input_lattice);
        }
        else
        {
            amrex::Vector<SlipSystem> a_slipsystem;
            a_slipsystem.resize(number_of_slip_systems);
            for (int i = 0; i<number_of_slip_systems; i++)
            {
                a_slipsystem[i].on = true;
                a_slipsystem[i].n = input_slipplanes[i];
                a_slipsystem[i].s = input_slipdirections[i];
                a_slipsystem[i].tcrss = input_tcrss[i];
                a_slipsystem[i].tcrss0 = input_tcrss[i];
                a_slipsystem[i].s_exp = input_exp[i];
                a_slipsystem[i].gam0dot = input_gam0dot[i];
            }
            value.Define(C11,C12,C44,m,a_slipsystem);
        }
    }
    AMREX_FORCE_INLINE
	void operator += (const CrystalPlasticDegradable &rhs)
    {
        curr += rhs.curr;
        prev += rhs.prev;
        for (int i = 0; i< std::min(rhs.slip.size(),slip.size()); i++) slip[i] += rhs.slip[i];
        ddw += rhs.ddw;
        F0  += rhs.F0;
        _F0  += rhs._F0;
        R += rhs.R;
        C110 += rhs.C110;
        C120 += rhs.C120;
        C440 += rhs.C440;
    }
    AMREX_FORCE_INLINE
    CrystalPlasticDegradable operator * (Set::Scalar alpha)
    {
        CrystalPlasticDegradable ret;
        ret.curr = curr*alpha;
        ret.prev = prev*alpha;
        ret.slip.resize(slip.size());
        for (int i=0; i<slip.size(); i++) ret.slip[i] = slip[i]*alpha;
        ret.ddw = ddw*alpha;
        ret.F0  = alpha*F0;
        ret._F0  = alpha*_F0;
        ret.R = alpha*R;
        ret.C110 = alpha*C110;
        ret.C120 = alpha*C120;
        ret.C440 = alpha*C440;
        return ret;
    }
    friend CrystalPlasticDegradable operator * (const Set::Scalar alpha, const CrystalPlasticDegradable b);
    friend CrystalPlasticDegradable operator + (const CrystalPlasticDegradable a, const CrystalPlasticDegradable b);
    friend CrystalPlasticDegradable operator - (const CrystalPlasticDegradable a, const CrystalPlasticDegradable b);
};
AMREX_FORCE_INLINE
CrystalPlasticDegradable operator * (const Set::Scalar alpha, const CrystalPlasticDegradable b)
{
    CrystalPlasticDegradable ret;
    ret.curr = b.curr*alpha;
    ret.prev = b.prev*alpha;
    ret.slip.resize(b.slip.size());
    for (int i=0; i<b.slip.size(); i++) ret.slip[i] = b.slip[i]*alpha;

    ret.ddw = b.ddw*alpha;
    ret.F0  = alpha*b.F0;
    ret._F0  = alpha*b._F0;
    ret.R = alpha*b.R;
    ret.C110 = alpha*b.C110;
    ret.C120 = alpha*b.C120;
    ret.C440 = alpha*b.C440;
    return ret;
}

AMREX_FORCE_INLINE
CrystalPlasticDegradable operator + (const CrystalPlasticDegradable a, const CrystalPlasticDegradable b)
{
    CrystalPlasticDegradable ret;
    ret.curr = a.curr + b.curr;
    ret.prev = a.prev + b.prev;
    ret.slip.resize(std::min(a.slip.size(),b.slip.size()));
    for (int i=0; i< std::min(a.slip.size(),b.slip.size()); i++) ret.slip[i] = a.slip[i] + b.slip[i];

    ret.ddw = a.ddw + b.ddw;
    ret.F0  = a.F0 + b.F0;
    ret._F0  = a._F0 + b._F0;
    ret.R = a.R + b.R;
    ret.C110 = a.C110 + b.C110;
    ret.C120 = a.C120 + b.C120;
    ret.C440 = a.C440 + b.C440;
    return ret;
}

AMREX_FORCE_INLINE
CrystalPlasticDegradable operator - (const CrystalPlasticDegradable a, const CrystalPlasticDegradable b)
{
    CrystalPlasticDegradable ret;
    ret.curr = a.curr - b.curr;
    ret.prev = a.prev - b.prev;
    ret.slip.resize(std::min(a.slip.size(),b.slip.size()));
    for (int i=0; i< std::min(a.slip.size(),b.slip.size()); i++) ret.slip[i] = a.slip[i] - b.slip[i];

    ret.ddw = a.ddw - b.ddw;
    ret.F0  = a.F0 - b.F0;
    ret._F0  = a._F0 - b._F0;
    ret.R = a.R - b.R;
    ret.C110 = a.C110 - b.C110;
    ret.C120 = a.C120 - b.C120;
    ret.C440 = a.C440 - b.C440;
    return ret;
}

}
}
}

#endif
