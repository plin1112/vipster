#ifndef GLWRAPPER_H
#define GLWRAPPER_H

#include <string>
#ifdef __EMSCRIPTEN__
#include <GLES3/gl3.h>
#else
#include <QOpenGLFunctions_3_3_Core>
#endif
#include <vector>
#include <array>
#include <set>
#include "molecule.h"

namespace Vipster {

typedef std::array<float,16> guiMat;

namespace Enums{
enum GuiChange{atoms=0x1, cell=0x2, fmt=0x4, kpoints=0x8, selection=0x10, settings=0x20};
}
using Enums::GuiChange;
constexpr auto guiStepChanged = GuiChange::atoms | GuiChange::cell | GuiChange::fmt | GuiChange::selection;
constexpr auto guiMolChanged = GuiChange::kpoints;

#ifdef __EMSCRIPTEN__
class GuiWrapper{
#else
class GuiWrapper: protected QOpenGLFunctions_3_3_Core{
#endif
    void loadShader(GLuint &program, const std::string &header, std::string vertShaderStr, std::string fragShaderStr);
public:
    void initGL(void);
    void initShaders(const std::string& header, const std::string& folder);
    void deleteGLObjects(void);
    void draw(void);
    void drawCell(void);
    void drawMol(void);
    void drawSel(void);
    // atom/bond/cell-data
    void initAtomVAO(void);
    void initBondVAO(void);
    void initCellVAO(void);
    void initSelVAO(void);
    void updateStepBuffers(StepProper* step, bool draw_bonds=true);
    void updateSelBuffers(StepSelection* sel);
    void updateVBOs(void);
    // view/projection matrices
    void initViewUBO(void);
    void updateViewUBO(void);
    void initViewMat(void);
    void resizeViewMat(int w, int h);
    void zoomViewMat(int i);
    void rotateViewMat(float x, float y, float z);
    void translateViewMat(float x, float y, float z);
    enum class alignDir{x,y,z,mx,my,mz};
    void alignViewMat(alignDir d);
    Mat getAxes();
    // molecule-store
    StepProper* curStep{nullptr};
    StepSelection* curSel{nullptr};
    // cpu-side data
    std::array<uint8_t,3> mult{{1,1,1}};
private:
    // separate change-flag in step for coord and rest!
    struct atom_prop{ // 8 bytes + 12 bytes directly from step
        float rad;  // 4 bytes
        ColVec col; // 4 bytes
    };
    std::vector<atom_prop> atom_prop_buffer{};
    struct sel_prop{ // 16 bytes
        Vec pos;    // 3*4 = 12 bytes
        float rad;  // 4 bytes
    };
    std::vector<sel_prop> sel_buffer{};
    struct bond_prop{ // 64 bytes
        float mat[9]; // 9*4 = 36 bytes
        Vec pos; // 3*4 = 12 bytes
        uint16_t mult[4];  // 4*2 = 6 bytes
        ColVec col_a, col_b; // 2*4 = 8 bytes
    };
    std::vector<bond_prop> bond_buffer{};
    std::array<Vec,8> cell_buffer{};
    std::array<float, 9>  cell_mat{};
    bool atoms_changed{false}, sel_changed{false};
    bool bonds_changed{false}, cell_changed{false};
    bool bonds_drawn = false;
    // gpu-side data
    GLuint atom_program, bond_program, cell_program, sel_program;
    GLuint atom_vao, bond_vao, cell_vao, sel_vao;
    GLuint atom_pos_vbo, atom_prop_vbo;
    GLuint bond_vbo, cell_vbo, sel_vbo;
    GLuint sphere_vbo, torus_vbo;
    GLuint cell_ibo;
    GLuint view_ubo;
    // cpu-side uniforms
    guiMat vMat, pMat, rMat;
    bool vMatChanged, pMatChanged, rMatChanged;
};

void guiMatScale(guiMat &m, float f);
void guiMatTranslate(guiMat &m, float x, float y, float z);
void guiMatRot(guiMat &m, float a, float x, float y, float z);
guiMat guiMatMkOrtho(float left, float right, float bottom, float top, float near, float far);
guiMat guiMatMkLookAt(Vec eye, Vec target, Vec up);
guiMat operator *=(guiMat &a, const guiMat &b);
guiMat operator *(guiMat a, const guiMat &b);

}

#endif // GLWRAPPER_H
