#include "lmptrajec.h"

#include <sstream>

using namespace Vipster;

auto IdentifyColumns(std::string& line)
{
    std::stringstream ss{line};
    std::string tok;
    //throw away "ITEM: ATOMS"
    ss >> tok >> tok;
    enum coordstate{none=0, unscaled=1, scaled=2, unwrapped=4};
    coordstate cs{};
    auto xparser = [](std::stringstream& ss, Atom& at) {ss >> at.coord[0];};
    auto yparser = [](std::stringstream& ss, Atom& at) {ss >> at.coord[1];};
    auto zparser = [](std::stringstream& ss, Atom& at) {ss >> at.coord[2];};
    auto nparser = [](std::stringstream& ss, Atom& at) {ss >> at.name;};
    auto qparser = [](std::stringstream& ss, Atom& at) {ss >> at.properties->charge;};
    auto dparser = [](std::stringstream& ss, Atom&) {static std::string dummy{}; ss >> dummy;};
    std::vector<void(*)(std::stringstream&, Atom&)> funvec{};
    while ( !(ss >> tok).fail() ) {
        if (tok[0] == 'x' || tok[0] == 'y' || tok[0] == 'z') {
            if ((tok.length() == 1) || (tok[1] != 's')) {
                cs = static_cast<coordstate>(cs | unscaled);
            } else if (tok[1] == 's') {
                cs = static_cast<coordstate>(cs | scaled);
            }
            if (((tok.length() == 2) && (tok[1] == 'u')) || (tok.length() == 3)) {
                cs = static_cast<coordstate>(cs | unwrapped);
            }
            if(tok[0] == 'x') {
                funvec.push_back(xparser);
            } else if(tok[0] == 'y') {
                funvec.push_back(yparser);
            } else if(tok[0] == 'z') {
                funvec.push_back(zparser);
            }
        } else if (tok == "q") {
            funvec.push_back(qparser);
        } else if (tok == "element") {
            funvec.push_back(nparser);
        } else {
            funvec.push_back(dparser);
        }
    }
    switch (static_cast<size_t>(cs)) {
    case unscaled:
        [[fallthrough]];
    case scaled:
        return [=](std::ifstream &file, StepProper& s){
            if(cs == scaled) {
                s.setFmt(AtomFmt::Crystal, true);
            } else {
                s.setFmt(AtomFmt::Angstrom, true);
            }
            std::string line;
            for (auto& at:s) {
                std::getline(file, line);
                std::stringstream ss{line};
                for (auto& fun:funvec) {
                    fun(ss, at);
                }
                if(ss.fail()) {
                    throw IO::Error("Lammps Dump: failed to parse atom");
                }
            }
        };
    case unscaled|scaled:
        throw IO::Error("Lammps Dump: mixed coordinates not supported");
    case none:
        [[fallthrough]];
    default:
        throw IO::Error("Lammps Dump: no coordinates present");
    }
}

IO::Data
LmpTrajecParser(const std::string& name, std::ifstream &file)
{
    enum class ParseMode{Header, Cell, Atoms};

    IO::Data data{};
    data.fmt = IOFmt::DMP;
    Molecule& m = data.mol;
    m.setName(name);
    StepProper* s = nullptr;

    std::string line;
    size_t nat;
    float t1, t2;
    Mat cell;
    while (std::getline(file, line)) {
        if (line.find("TIMESTEP") != std::string::npos) {
            s = &m.newStep();
            // skip 2 lines
            std::getline(file, line);
            std::getline(file, line);
            // Number of Atoms
            std::getline(file, line);
            std::stringstream ss{line};
            ss >> nat;
            if (ss.fail()) {
                throw IO::Error("Lammps Dump: failed to parse nat");
            }
            s->newAtoms(nat);
            s->setCellDim(1, CdmFmt::Angstrom);
            // Cell
            cell = Mat{};
            std::getline(file, line);
            if ((line.length() > 17) && (line[17] == 'x')) {
                // xlo, xhi, xy
                std::getline(file, line);
                ss = std::stringstream{line};
                ss >> t1 >> t2 >> cell[1][0];
                cell[0][0] = t2 - t1;
                // ylo, yhi, xz
                std::getline(file, line);
                ss = std::stringstream{line};
                ss >> t1 >> t2 >> cell[2][0];
                cell[1][1] = t2 - t1;
                // zlo, zhi, yz
                std::getline(file, line);
                ss = std::stringstream{line};
                ss >> t1 >> t2 >> cell[2][1];
                cell[2][2] = t2 - t1;
            } else {
                // xlo, xhi
                std::getline(file, line);
                ss = std::stringstream{line};
                ss >> t1 >> t2;
                cell[0][0] = t2 - t1;
                // ylo, yhi
                std::getline(file, line);
                ss = std::stringstream{line};
                ss >> t1 >> t2;
                cell[1][1] = t2 - t1;
                // zlo, zhi
                std::getline(file, line);
                ss = std::stringstream{line};
                ss >> t1 >> t2;
                cell[2][2] = t2 - t1;
            }
            if (ss.fail()) {
                throw IO::Error("Lammps Dump: failed to parse box");
            }
            s->setCellVec(cell, (s->getFmt()==AtomFmt::Crystal));
            // Atoms
            std::getline(file, line);
            auto atomParser = IdentifyColumns(line);
            atomParser(file, *s);
        }
    }
    return data;
}

const IO::Plugin IO::LmpTrajec =
{
    "Lammps Dump",
    "lammpstrj",
    "dmp",
    IO::Plugin::None,
    &LmpTrajecParser,
    nullptr
};
