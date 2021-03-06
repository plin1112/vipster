#ifndef BONDCELLINTERFACE_H
#define BONDCELLINTERFACE_H

#include "atom.h"
#include "bond.h"
#include "cell.h"
#include "vec.h"
#include "config.h"

#include <memory>
#include <vector>
#include <set>
#include <functional>
#include <algorithm>

namespace Vipster {

// Bond container
struct BondList{
    bool                outdated{true}, setOnce{false};
    BondLevel           level{BondLevel::None};
    float               cutoff_factor{settings.bondCutFac.val};
    std::vector<Bond>   bonds;
};

/*
 * Base for all Step-like containers
 *
 * Uses CRTP to access Atoms and cache
 * Implements interface for Bonds, Cell, Comment
 */
template<typename T>
class StepBase
{
public:
    virtual ~StepBase()=default;

    // Don't know how to mask this yet
    std::shared_ptr<PseMap> pse;

    // Comment
    void                setComment(const std::string& s)
    {
        *comment = s;
    }
    const std::string&  getComment() const noexcept
    {
        return *comment;
    }

    // Types
    std::set<std::string>   getTypes() const
    {
        const auto& names{static_cast<const T*>(this)->getNames()};
        return std::set<std::string>{names.begin(), names.end()};
    }
    size_t                  getNtyp() const
    {
        return getTypes().size();
    }

    // Format
    AtomFmt getFmt() const noexcept{
        return at_fmt;
    }
    virtual T&          asFmt(AtomFmt)=0;
    virtual const T&    asFmt(AtomFmt) const=0;
    std::function<Vec(const Vec&)>  getFormatter(AtomFmt source,
                                                 AtomFmt target) const
    {
        float fac{};
        Mat fmat{};
        switch(source) {
        case AtomFmt::Bohr:
            switch(target){
            case AtomFmt::Angstrom:
                return [](const Vec& v){return v*Vipster::bohrrad;};
            case AtomFmt::Alat:
                fac = 1/getCellDim(CdmFmt::Bohr);
                return [fac](const Vec& v){return v*fac;};
            case AtomFmt::Crystal:
                fmat = Mat_inv(getCellVec())/getCellDim(CdmFmt::Bohr);
                return [fmat](const Vec& v){return v*fmat;};
            default:
                break;
            }
            break;
        case AtomFmt::Angstrom:
            switch(target){
            case AtomFmt::Bohr:
                return [](const Vec& v){return v*Vipster::invbohr;};
            case AtomFmt::Alat:
                fac = 1/getCellDim(CdmFmt::Angstrom);
                return [fac](const Vec& v){return v*fac;};
            case AtomFmt::Crystal:
                fmat = Mat_inv(getCellVec())/getCellDim(CdmFmt::Angstrom);
                return [fmat](const Vec& v){return v*fmat;};
            default:
                break;
            }
            break;
        case AtomFmt::Alat:
            switch(target){
            case AtomFmt::Angstrom:
                fac = getCellDim(CdmFmt::Angstrom);
                return [fac](const Vec& v){return v*fac;};
            case AtomFmt::Bohr:
                fac = getCellDim(CdmFmt::Bohr);
                return [fac](const Vec& v){return v*fac;};
            case AtomFmt::Crystal:
                fmat = Mat_inv(getCellVec());
                return [fmat](const Vec& v){return v*fmat;};
            default:
                break;
            }
            break;
        case AtomFmt::Crystal:
            switch(target){
            case AtomFmt::Angstrom:
                fmat = getCellVec()*getCellDim(CdmFmt::Angstrom);
                return [fmat](const Vec& v){return v*fmat;};
            case AtomFmt::Alat:
                fmat = getCellVec();
                return [fmat](const Vec& v){return v*fmat;};
            case AtomFmt::Bohr:
                fmat = getCellVec()*getCellDim(CdmFmt::Bohr);
                return [fmat](const Vec& v){return v*fmat;};
            default:
                break;
            }
        }
        return [](const Vec& v){return v;};
    }
    Vec formatVec(Vec in, AtomFmt source, AtomFmt target) const
    {
        return getFormatter(source, target)(in);
    }
    std::vector<Vec> formatAll(std::vector<Vec> in, AtomFmt source,
                               AtomFmt target) const
    {
        if ((source == target) || in.empty()){
            return in;
        }
        auto op = getFormatter(source, target);
        std::transform(in.begin(), in.end(), in.begin(), op);
        return in;
    }

    // Bonds
    const std::vector<Bond>&    getBonds(BondLevel l=BondLevel::Cell,
                                         BondFrequency update=BondFrequency::Always) const
    {
        return getBonds(bonds->cutoff_factor, l, update);
    }
    const std::vector<Bond>&    getBonds(float cutfac,
                                         BondLevel l=BondLevel::Cell,
                                         BondFrequency update=BondFrequency::Always) const
    {
        evaluateCache();
        if(((update == BondFrequency::Always) or
            ((update == BondFrequency::Once) and not bonds->setOnce))
           and
           (bonds->outdated or
            (float_comp(cutfac, bonds->cutoff_factor)) or
            (bonds->level < l)))
        {
            setBonds(l, cutfac);
        }
        return bonds->bonds;
    }
    size_t                      getNbond() const
    {
        return getBonds().size();
    }
    void    setBonds(BondLevel l) const
    {
        setBonds(l, settings.bondCutFac.val);
    }
    void    setBonds(BondLevel l, float cutfac) const
    {
        bonds->bonds.clear();
        if(!static_cast<const T*>(this)->getNat()){
            l = BondLevel::None;
        }
        if((l==BondLevel::Cell) && !cell->enabled){ l = BondLevel::Molecule; }
        switch(l){
        case BondLevel::None:
            break;
        case BondLevel::Molecule:
            setBondsMolecule(cutfac);
            break;
        case BondLevel::Cell:
            setBondsCell(cutfac);
            break;
        }
        bonds->outdated = false;
        bonds->setOnce = true;
        bonds->cutoff_factor = cutfac;
        bonds->level = l;
    }

    // Cell
    bool    hasCell() const noexcept
    {
        return cell->enabled;
    }

    float   getCellDim(CdmFmt fmt) const noexcept
    {
        if (fmt == CdmFmt::Bohr) {
            return cell->dimBohr;
        }
        return cell->dimAngstrom;
    }

    Mat     getCellVec() const noexcept
    {
        return cell->cellvec;
    }

    Vec     getCom(AtomFmt fmt) const noexcept
    {
        if(!static_cast<const T*>(this)->getNat()){
            return Vec{{0,0,0}};
        }
        Vec min{{std::numeric_limits<float>::max(),
                 std::numeric_limits<float>::max(),
                 std::numeric_limits<float>::max()}};
        Vec max{{std::numeric_limits<float>::min(),
                 std::numeric_limits<float>::min(),
                 std::numeric_limits<float>::min()}};
        for(auto& at:*static_cast<const T*>(this)){
            min[0]=std::min(min[0],at.coord[0]);
            min[1]=std::min(min[1],at.coord[1]);
            min[2]=std::min(min[2],at.coord[2]);
            max[0]=std::max(max[0],at.coord[0]);
            max[1]=std::max(max[1],at.coord[1]);
            max[2]=std::max(max[2],at.coord[2]);
        }
        return formatVec((min+max)/2, at_fmt, fmt);
    }

    Vec     getCenter(CdmFmt fmt, bool com=false) const noexcept
    {
        if(com || !cell->enabled){
            return getCom(static_cast<AtomFmt>(fmt));
        }
        const Mat& cv = cell->cellvec;
        return (cv[0]+cv[1]+cv[2]) * getCellDim(fmt) / 2;
    }

    virtual void evaluateCache()const =0;

    // Modifier functions
    void modShift(Vec shift, float fac=1.0f){
        shift *= fac;
        for(Atom& at:*static_cast<T*>(this)){
            at.coord += shift;
        }
    }

    void modRotate(float angle, Vec axis, Vec shift={0,0,0}){
        angle *= deg2rad;
        float c = std::cos(angle);
        float s = -std::sin(angle);
        float ic = 1.f-c;
        float len = Vec_length(axis);
        if(float_comp(len, 0.f)){
            throw Error("0-Vector cannot be rotation axis");
        }
        axis /= len;
        Mat rotMat = {Vec{ic * axis[0] * axis[0] + c,
                          ic * axis[0] * axis[1] - s * axis[2],
                          ic * axis[0] * axis[2] + s * axis[1]},
                      Vec{ic * axis[1] * axis[0] + s * axis[2],
                          ic * axis[1] * axis[1] + c,
                          ic * axis[1] * axis[2] - s * axis[0]},
                      Vec{ic * axis[2] * axis[0] - s * axis[1],
                          ic * axis[2] * axis[1] + s * axis[0],
                          ic * axis[2] * axis[2] + c}};
        for(Atom& at:*static_cast<T*>(this)){
            at.coord = (at.coord - shift) * rotMat + shift;
        }
    }

    void modMirror(Vec ax1, Vec ax2, Vec shift={0,0,0}){
        Vec normal = Vec_cross(ax1, ax2);
        normal /= Vec_length(normal);
        for(Atom& at:*static_cast<T*>(this)){
            at.coord -= 2*Vec_dot(at.coord-shift, normal)*normal;
        }
    }

    void modWrap(){
        for(Atom& at:asFmt(AtomFmt::Crystal)){
            at.coord[0] -= std::floor(at.coord[0]);
            at.coord[1] -= std::floor(at.coord[1]);
            at.coord[2] -= std::floor(at.coord[2]);
        }
    }

    void modCrop(){
        std::vector<size_t> toRemove;
        toRemove.reserve(static_cast<T*>(this)->getNat());
        for(auto it=asFmt(AtomFmt::Crystal).begin(); it!=asFmt(AtomFmt::Crystal).end(); ++it){
            if((it->coord[0]>=1) | (it->coord[0]<0) |
               (it->coord[1]>=1) | (it->coord[1]<0) |
               (it->coord[2]>=1) | (it->coord[2]<0)){
                toRemove.push_back(it.getIdx());
            }
        }
        for(auto it=toRemove.rbegin(); it!=toRemove.rend(); ++it){
            static_cast<T*>(this)->delAtom(*it);
        }
    }

    void modMultiply(size_t x, size_t y, size_t z){
        auto fac = x*y*z;
        if(fac == 0){
            throw Error("Cannot eradicate atoms via modMultiply");
        }else if(fac == 1){
            return;
        }
        T& handle = static_cast<T*>(this)->asFmt(AtomFmt::Crystal);
        auto cell = getCellVec();
        auto multiply = [&](uint8_t dir, uint8_t mult){
            auto atoms = handle.getAtoms();
            auto oldNat = handle.getNat();
            cell[dir] *= mult;
            for(uint8_t i=1; i<mult; ++i){
                handle.newAtoms(atoms);
                auto refIt = handle.begin();
                for(auto it=refIt+i*oldNat; it!=refIt+(i+1)*oldNat; ++it){
                    it->coord[dir] += i;
                }
            }
        };
        if(x>1){
            multiply(0, x);
        }
        if(y>1){
            multiply(1, y);
        }
        if(z>1){
            multiply(2, z);
        }
        static_cast<T*>(this)->setCellVec(cell);
    }

    void modAlign(uint8_t step_dir, uint8_t target_dir){
        auto target = Vec{};
        target.at(target_dir) = 1;
        auto source = getCellVec().at(step_dir);
        source /= Vec_length(source);
        if(target == source){
            return;
        }
        auto axis = Vec_cross(source, target);
        axis /= Vec_length(axis);
        auto cos = Vec_dot(source, target);
        auto icos = 1-cos;
        auto sin = -std::sqrt(1-cos*cos);
        Mat rotMat = {Vec{icos * axis[0] * axis[0] + cos,
                          icos * axis[0] * axis[1] - sin * axis[2],
                          icos * axis[0] * axis[2] + sin * axis[1]},
                      Vec{icos * axis[1] * axis[0] + sin * axis[2],
                          icos * axis[1] * axis[1] + cos,
                          icos * axis[1] * axis[2] - sin * axis[0]},
                      Vec{icos * axis[2] * axis[0] - sin * axis[1],
                          icos * axis[2] * axis[1] + sin * axis[0],
                          icos * axis[2] * axis[2] + cos}};
        Mat oldCell = getCellVec();
        Mat newCell = oldCell*rotMat;
        static_cast<T*>(this)->setCellVec(newCell, true);
    }

    void modReshape(Mat newMat, float newCdm, CdmFmt cdmFmt){
        auto oldCdm = getCellDim(cdmFmt);
        auto oldMat = getCellVec();
        if((newMat == oldMat) && (float_comp(newCdm, oldCdm))){
            return;
        }
        modWrap();
        auto& handle = *static_cast<T*>(this);
        size_t fac;
        if(newMat == oldMat){
            // only changing cdm
            fac = std::ceil(newCdm/oldCdm);
        }else{
            // change vectors or both
            auto getExtent = [](const Mat& m){
                return Vec{m[0][0] + m[1][0] + m[2][0],
                           m[0][1] + m[1][1] + m[2][1],
                           m[0][2] + m[1][2] + m[2][2]
                           };
            };
            auto compExtLt = [](const Vec& v1, const Vec& v2){
                return (v1[0] < v2[0]) || (v1[1] < v2[1]) || (v1[2] < v2[2]);
            };
            Vec newExtent = getExtent(newMat*newCdm);
            Vec oldExtent = getExtent(oldMat*oldCdm);
            fac = 1;
            while(compExtLt(oldExtent*fac, newExtent)){
                fac += 1;
            }
        }
        modMultiply(fac, fac, fac);
        handle.setCellVec(newMat);
        handle.setCellDim(newCdm, cdmFmt);
        modCrop();
    }

protected:
    StepBase(std::shared_ptr<PseMap> pse, AtomFmt fmt, std::shared_ptr<BondList> bonds,
             std::shared_ptr<CellData> cell, std::shared_ptr<std::string> comment)
        : pse{pse}, at_fmt{fmt}, bonds{bonds}, cell{cell}, comment{comment} {}
    // Data
    AtomFmt                         at_fmt;
    std::shared_ptr<BondList>       bonds;
    std::shared_ptr<CellData>       cell;
    std::shared_ptr<std::string>    comment;

private:
    // Bonds
    void    setBondsMolecule(float cutfac) const
    {
        AtomFmt fmt = (this->at_fmt == AtomFmt::Angstrom) ? AtomFmt::Angstrom : AtomFmt::Bohr;
        float fmtscale{(fmt == AtomFmt::Angstrom) ? invbohr : 1};
        const T& tgtFmt = asFmt(fmt);
        tgtFmt.evaluateCache();
        std::vector<Bond>& bonds = this->bonds->bonds;
        auto at_i = tgtFmt.begin();
        for (auto at_i=tgtFmt.begin(); at_i!=tgtFmt.end(); ++at_i)
        {
            float cut_i = at_i->pse->bondcut;
            if (cut_i<0){
                continue;
            }
            for (auto at_j=tgtFmt.begin()+at_i.getIdx()+1; at_j != tgtFmt.end(); ++at_j){
                float cut_j = at_j->pse->bondcut;
                if (cut_j<0) {
                    continue;
                }
                float effcut = (cut_i + cut_j) * cutfac;
                Vec dist_v = at_i->coord - at_j->coord;
                if (((dist_v[0] *= fmtscale) > effcut) ||
                    ((dist_v[1] *= fmtscale) > effcut) ||
                    ((dist_v[2] *= fmtscale) > effcut)) {
                    continue;
                }
                float dist_n = Vec_dot(dist_v, dist_v);
                if((0.57f < dist_n) && (dist_n < effcut*effcut)) {
                    bonds.push_back({at_i.getIdx(), at_j.getIdx(), std::sqrt(dist_n), 0, 0, 0});
                }
            }
        }
        this->bonds->outdated = false;
        this->bonds->level = BondLevel::Molecule;
    }

    void checkBond(std::size_t i, std::size_t j, float effcut,
                   const Vec& dist, const std::array<int16_t, 3>& offset) const
    {
        auto& bonds = this->bonds->bonds;
        if ((dist[0]>effcut) || (dist[1]>effcut) || (dist[2]>effcut)) {
            return;
        }
        float dist_n = Vec_dot(dist, dist);
        if ((0.57f < dist_n) && (dist_n < effcut*effcut)) {
            bonds.push_back({i, j, std::sqrt(dist_n), offset[0], offset[1], offset[2]});
        }
    }

    void setBondsCell(float cutfac) const
    {
        const T& asCrystal = asFmt(AtomFmt::Crystal);
        asCrystal.evaluateCache();
        const Vec x = getCellVec()[0] * getCellDim(CdmFmt::Bohr);
        const Vec y = getCellVec()[1] * getCellDim(CdmFmt::Bohr);
        const Vec z = getCellVec()[2] * getCellDim(CdmFmt::Bohr);
        const Vec xy   = x+y;
        const Vec xmy  = x-y;
        const Vec xz   = x+z;
        const Vec xmz  = x-z;
        const Vec yz   = y+z;
        const Vec ymz  = y-z;
        const Vec xyz  = xy + z;
        const Vec xymz = xy - z;
        const Vec xmyz = xz - y;
        const Vec mxyz = yz - x;
        std::array<int16_t, 3> diff_v, crit_v;
        size_t nat = static_cast<const T*>(this)->getNat();
        auto at_i = asCrystal.begin();
        for (size_t i=0; i<nat; ++i) {
            float cut_i = at_i->pse->bondcut;
            if (cut_i<0) {
                continue;
            }
            auto at_j = asCrystal.begin();
            for (size_t j=0; j<nat; ++j) {
                float cut_j = at_j->pse->bondcut;
                if (cut_j<0) {
                    continue;
                }
                float effcut = (cut_i + cut_j) * cutfac;
                Vec dist_v = at_i->coord - at_j->coord;
                std::transform(dist_v.begin(), dist_v.end(), diff_v.begin(), truncf);
                std::transform(dist_v.begin(), dist_v.end(), dist_v.begin(),
                    [](float f){return std::fmod(f,1);});
                std::transform(dist_v.begin(), dist_v.end(), crit_v.begin(),
                    [](float f){
                        return (std::abs(f) < std::numeric_limits<float>::epsilon())?
                                    0 : ((f<0) ? -1 : 1);
                    });
                if(!((crit_v[0] != 0)||(crit_v[1] != 0)||(crit_v[2] != 0))){
                    // TODO: fail here? set flag? overlapping atoms!
                    continue;
                }
                dist_v = dist_v * cell->cellvec * cell->dimBohr;
                // 0-vector
                checkBond(i, j, effcut, dist_v, diff_v);
                if(crit_v[0] != 0){
                    // x, -x
                    checkBond(i, j, effcut, dist_v-crit_v[0]*x,
                              {{static_cast<int16_t>(diff_v[0]+crit_v[0]),diff_v[1],diff_v[2]}});
                }
                if(crit_v[1] != 0){
                    // y, -y
                    checkBond(i, j, effcut, dist_v-crit_v[1]*y,
                              {{diff_v[0],static_cast<int16_t>(diff_v[1]+crit_v[1]),diff_v[2]}});
                    if(crit_v[0] != 0){
                        if(crit_v[0] == crit_v[1]){
                            // x+y, -x-y
                            checkBond(i, j, effcut, dist_v-crit_v[0]*xy,
                                      {{static_cast<int16_t>(diff_v[0]+crit_v[0]),
                                        static_cast<int16_t>(diff_v[1]+crit_v[1]),
                                        diff_v[2]}});
                        }else{
                            // x-y, -x+y
                            checkBond(i, j, effcut, dist_v-crit_v[0]*xmy,
                                      {{static_cast<int16_t>(diff_v[0]+crit_v[0]),
                                        static_cast<int16_t>(diff_v[1]+crit_v[1]),
                                        diff_v[2]}});
                        }
                    }
                }
                if(crit_v[2] != 0){
                    // z, -z
                    checkBond(i, j, effcut, dist_v-crit_v[2]*z,
                              {{diff_v[0],diff_v[1],static_cast<int16_t>(diff_v[2]+crit_v[2])}});
                    if(crit_v[0] != 0){
                        if(crit_v[0] == crit_v[2]){
                            // x+z, -x-z
                            checkBond(i, j, effcut, dist_v-crit_v[0]*xz,
                                      {{static_cast<int16_t>(diff_v[0]+crit_v[0]),
                                        diff_v[1],
                                        static_cast<int16_t>(diff_v[2]+crit_v[2])}});
                        }else{
                            // x-z, -x+z
                            checkBond(i, j, effcut, dist_v-crit_v[0]*xmz,
                                      {{static_cast<int16_t>(diff_v[0]+crit_v[0]),
                                        diff_v[1],
                                        static_cast<int16_t>(diff_v[2]+crit_v[2])}});
                        }
                    }
                    if(crit_v[1] != 0){
                        if(crit_v[1] == crit_v[2]){
                            // y+z, -y-z
                            checkBond(i, j, effcut, dist_v-crit_v[1]*yz,
                                      {{diff_v[0],
                                        static_cast<int16_t>(diff_v[1]+crit_v[1]),
                                        static_cast<int16_t>(diff_v[2]+crit_v[2])}});
                        }else{
                            // y-z, -y+z
                            checkBond(i, j, effcut, dist_v-crit_v[1]*ymz,
                                      {{diff_v[0],
                                        static_cast<int16_t>(diff_v[1]+crit_v[1]),
                                        static_cast<int16_t>(diff_v[2]+crit_v[2])}});
                        }
                        if(crit_v[0] != 0){
                            if(crit_v[0] == crit_v[1]){
                                if(crit_v[0] == crit_v[2]){
                                    // x+y+z, -x-y-z
                                    checkBond(i, j, effcut, dist_v-crit_v[0]*xyz,
                                              {{static_cast<int16_t>(diff_v[0]+crit_v[0]),
                                                static_cast<int16_t>(diff_v[1]+crit_v[1]),
                                                static_cast<int16_t>(diff_v[2]+crit_v[2])}});
                                }else{
                                    // x+y-z, -x-y+z
                                    checkBond(i, j, effcut, dist_v-crit_v[0]*xymz,
                                              {{static_cast<int16_t>(diff_v[0]+crit_v[0]),
                                                static_cast<int16_t>(diff_v[1]+crit_v[1]),
                                                static_cast<int16_t>(diff_v[2]+crit_v[2])}});
                                }
                            }else{
                                if(crit_v[0] == crit_v[2]){
                                    // x-y+z, -x+y-z
                                    checkBond(i, j, effcut, dist_v-crit_v[0]*xmyz,
                                              {{static_cast<int16_t>(diff_v[0]+crit_v[0]),
                                                static_cast<int16_t>(diff_v[1]+crit_v[1]),
                                                static_cast<int16_t>(diff_v[2]+crit_v[2])}});
                                }else{
                                    // x-y-z, -x+y+z
                                    checkBond(i, j, effcut, dist_v-crit_v[1]*mxyz,
                                              {{static_cast<int16_t>(diff_v[0]+crit_v[0]),
                                                static_cast<int16_t>(diff_v[1]+crit_v[1]),
                                                static_cast<int16_t>(diff_v[2]+crit_v[2])}});
                                }
                            }
                        }
                    }
                }
                ++at_j;
            }
            ++at_i;
        }
        bonds->outdated = false;
        bonds->level = BondLevel::Cell;
    }
};

}

#endif // BONDCELLINTERFACE_H
