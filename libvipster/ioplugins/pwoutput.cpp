#include "pwoutput.h"

#include <sstream>

using namespace Vipster;

std::shared_ptr<IO::BaseData> PWOutParser(std::string name, std::ifstream &file)
{
    auto d = std::make_shared<IO::BaseData>();
    Molecule& m = d->mol;
    m.setName(name);
    StepProper *s = &m.newStep();

    std::string line, dummy_s;
    size_t nat{}, ntype{};
    float celldim{};
    Mat cellvec;
    bool gamma{false};
    CdmFmt cdmfmt{CdmFmt::Bohr};
    while (std::getline(file, line)) {
        if (line.find("number of atoms/cell") != line.npos) {
            std::stringstream{line.substr(33)} >> nat;
            s->newAtoms(nat);
            std::getline(file, line);
            std::stringstream{line.substr(33)} >> ntype;
        } else if (line.find("gamma-point") != line.npos) {
            gamma = true;
        } else if (line.find("number of k points=") != line.npos) {
            if (gamma) {
                continue;
            }
            KPoints kpts{};
            size_t nk = static_cast<size_t>(std::stoi(line.substr(line.find('=')+1)));
            // skip header
            std::getline(file, line);
            if (line.find("cart. coord.") == line.npos){
                continue;
            }
            kpts.discrete.kpoints.resize(nk);
            for (auto& k: kpts.discrete.kpoints) {
                std::getline(file, line);
                std::stringstream ss{line};
                std::getline(ss, dummy_s, '(');
                std::getline(ss, dummy_s, '(');
                ss >> k.pos[0] >> k.pos[1] >> k.pos[2];
                std::getline(ss, dummy_s, '=');
                ss >> k.weight;
            }
        } else if (line.find("celldm(1)=") != line.npos) {
            /*
             * parse celldm(1)
             * always given with discrete vectors
             * ibrav and rest of celldm not needed
             */
            std::stringstream{line} >> dummy_s >> celldim;
            s->setCellDim(celldim, cdmfmt);
            // skip to cell-vectors
            std::getline(file, line);
            std::getline(file, line);
            std::getline(file, line);
            for(size_t i=0; i<3; ++i){
                std::getline(file, line);
                std::stringstream{line} >> dummy_s >> dummy_s >> dummy_s
                        >> cellvec[i][0] >> cellvec[i][1] >> cellvec[i][2];
            }
            s->setCellVec(cellvec);
        } else if (line.find("site n.") != line.npos) {
            // parse initial coordinates
            // always given as ALAT
            s->setFmt(AtomFmt::Alat);
            for(auto& at: *s){
                std::getline(file, line);
                std::stringstream ss{line};
                ss >> dummy_s >> at.name >> dummy_s;
                std::getline(ss, dummy_s, '(');
                ss >> at.coord[0] >> at.coord[1] >> at.coord[2];
                if (ss.fail()) {
                    throw IOError{"Failed to parse atom"};
                }
            }
        } else if (line.find("CELL_PARAMETERS") != line.npos) {
            if (line.find("(bohr)") != line.npos) {
                cdmfmt = CdmFmt::Bohr;
                celldim = 1;
            }else if (line.find("angstrom") != line.npos) {
                cdmfmt = CdmFmt::Angstrom;
                celldim = 1;
            }else{
                cdmfmt = CdmFmt::Bohr;
                celldim = std::stof(line.substr(line.find('=')+1));
            }
            // parse vectors
            for(Vec& v: cellvec){
                std::getline(file, line);
                std::stringstream{line} >> v[0] >> v[1] >> v[2];
            }
        } else if (line.find("ATOMIC_POSITIONS") != line.npos) {
            // formatted positions
            // creating new step here
            s = &m.newStep();
            s->setCellDim(celldim, cdmfmt);
            s->setCellVec(cellvec);
            s->newAtoms(nat);
            if(line.find("angstrom") != line.npos) {
                s->setFmt(AtomFmt::Angstrom);
            }else if (line.find("bohr") != line.npos) {
                s->setFmt(AtomFmt::Bohr);
            }else if (line.find("crystal") != line.npos) {
                s->setFmt(AtomFmt::Crystal);
            }else{
                s->setFmt(AtomFmt::Alat);
            }
            for(auto& at: *s){
                std::getline(file, line);
                std::stringstream{line} >> at.name
                        >> at.coord[0] >> at.coord[1] >> at.coord[2];
            }
        }
        else if (line.find("Begin final coordinates") != line.npos) {
            break;
        }
    }

    return d;
}

const IOPlugin IO::PWOutput =
{
    "PWScf Output File",
    "pwo",
    "pwo",
    &PWOutParser,
    nullptr
};