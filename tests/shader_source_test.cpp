// Plain test executable for the GL-free shader helpers in glshaders.h and for
// the shader sources themselves. No framework: each failing CHECK is printed
// and the process exits non-zero if any check failed.
//
// CANE_SHADER_TOOLS_ONLY keeps glshaders.h from pulling in the game headers.
// Built on macOS only, with CANE_GL41 defined: the shaders it scans are the
// ones the 4.1 path loads, gl41/<name> where that copy exists.

#include "../Sly1-Release/glshaders.h"

#include <CommonCrypto/CommonDigest.h>
#include <unistd.h>

#include <cctype>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <map>
#include <regex>
#include <set>
#include <sstream>
#include <string>
#include <vector>

#ifndef CANE_SHADER_DIR
#error "CANE_SHADER_DIR must name the directory holding the .vert/.frag/.geom files"
#endif

namespace
{
int g_failures = 0;
int g_checks = 0;

#define CHECK(condition) Check((condition), #condition, __FILE__, __LINE__)
#define CHECK_STR_EQ(left, right) CheckStrEqual((left), (right), #left, #right, __FILE__, __LINE__)

void Check(bool condition, const char* expression, const char* file, int line)
{
    ++g_checks;
    if (condition)
        return;
    ++g_failures;
    std::fprintf(stderr, "%s:%d: CHECK failed: %s\n", file, line, expression);
}

void CheckStrEqual(const std::string& left, const std::string& right,
    const char* leftText, const char* rightText, const char* file, int line)
{
    ++g_checks;
    if (left == right)
        return;
    ++g_failures;
    std::fprintf(stderr, "%s:%d: CHECK_STR_EQ failed: %s == %s\n    left:  [%s]\n    right: [%s]\n",
        file, line, leftText, rightText, left.c_str(), right.c_str());
}

void Fail(const std::string& message)
{
    ++g_checks;
    ++g_failures;
    std::fprintf(stderr, "FAIL: %s\n", message.c_str());
}

// ---- T1: version rewrite -------------------------------------------------

void TestRewriteReplacesLeading430Core()
{
    const std::string source = "#version 430 core\nlayout(location = 0) in vec3 p;\n";
    CHECK_STR_EQ(RewriteShaderVersion(source, "410 core"),
        "#version 410 core\nlayout(location = 0) in vec3 p;\n");
}

void TestRewriteLeaves330Untouched()
{
    const std::string source = "#version 330 core\nvoid main() {}\n";
    CHECK_STR_EQ(RewriteShaderVersion(source, "410 core"), source);
}

void TestRewriteLeavesLaterVersionTextUntouched()
{
    // Only the leading directive counts; text further down is never rewritten.
    const std::string source = "#version 330 core\n// was #version 430 core\n#version 430 core\n";
    CHECK_STR_EQ(RewriteShaderVersion(source, "410 core"), source);

    const std::string body = "void main() {}\n#version 430 core\n";
    CHECK_STR_EQ(RewriteShaderVersion(body, "410 core"), body);
}

void TestRewriteKeepsBomAndLeadingWhitespace()
{
    // The prefix is preserved byte for byte; only the directive changes.
    const std::string bom = "\xEF\xBB\xBF";
    CHECK_STR_EQ(RewriteShaderVersion(bom + "#version 430 core\nx\n", "410 core"),
        bom + "#version 410 core\nx\n");
    CHECK_STR_EQ(RewriteShaderVersion("\n  \t#version 430 core\nx\n", "410 core"),
        "\n  \t#version 410 core\nx\n");
}

void TestRewriteHandlesCrlfAndEndOfInput()
{
    CHECK_STR_EQ(RewriteShaderVersion("#version 430 core\r\nx\r\n", "410 core"),
        "#version 410 core\r\nx\r\n");
    CHECK_STR_EQ(RewriteShaderVersion("#version 430 core", "410 core"), "#version 410 core");
}

void TestRewriteRequiresWholeDirective()
{
    const std::string compat = "#version 430 compatibility\n";
    CHECK_STR_EQ(RewriteShaderVersion(compat, "410 core"), compat);
    const std::string longer = "#version 430 core_profile\n";
    CHECK_STR_EQ(RewriteShaderVersion(longer, "410 core"), longer);
    CHECK_STR_EQ(RewriteShaderVersion("", "410 core"), "");
}

// ---- T2: shader source scan ----------------------------------------------

struct ShaderFile
{
    std::string name;
    std::vector<std::string> lines;
};

bool IsShaderFile(const std::filesystem::path& path)
{
    const std::string extension = path.extension().string();
    return extension == ".vert" || extension == ".frag" || extension == ".geom";
}

ShaderFile ReadShader(const std::string& name, const std::filesystem::path& path)
{
    ShaderFile shader{ name, {} };
    std::ifstream stream(path, std::ios::binary);
    std::string line;
    while (std::getline(stream, line))
    {
        if (!line.empty() && line.back() == '\r')
            line.pop_back();
        shader.lines.push_back(line);
    }
    return shader;
}

// The shaders the GL 4.1 path loads: gl41/<name> when that copy exists (the
// build embeds it in place of the shared file), else the shared file, plus
// the Mac-only gl41 files that have no shared counterpart.
std::vector<ShaderFile> LoadShaders()
{
    const std::filesystem::path shaderDir(CANE_SHADER_DIR);
    std::vector<ShaderFile> shaders;
    for (const auto& entry : std::filesystem::directory_iterator(shaderDir))
    {
        if (!IsShaderFile(entry.path()))
            continue;
        const std::string name = entry.path().filename().string();
        std::filesystem::path path = shaderDir / "gl41" / name;
        if (!std::filesystem::exists(path))
            path = entry.path();
        shaders.push_back(ReadShader(name, path));
    }
    for (const auto& entry : std::filesystem::directory_iterator(shaderDir / "gl41"))
    {
        const std::string name = entry.path().filename().string();
        if (IsShaderFile(entry.path()) && !std::filesystem::exists(shaderDir / name))
            shaders.push_back(ReadShader(name, entry.path()));
    }
    return shaders;
}

bool InBindingTable(const std::string& block)
{
    for (const UniformBlockBinding& entry : kUniformBlockBindings)
    {
        if (block == entry.name)
            return true;
    }
    return false;
}

void TestShaderSources()
{
    const std::vector<ShaderFile> shaders = LoadShaders();
    CHECK(shaders.size() >= 15);

    const std::regex layoutLine(R"(^\s*layout\s*\(([^)]*)\))");
    const std::regex blockOnLine(R"(\buniform\s+(\w+)\s*(\{.*)?$)");
    int std140Blocks = 0;
    int std430Blocks = 0;
    int uniformBlocks = 0;

    for (const ShaderFile& shader : shaders)
    {
        for (size_t i = 0; i < shader.lines.size(); ++i)
        {
            const std::string& line = shader.lines[i];
            const std::string where = shader.name + ":" + std::to_string(i + 1);

            std::smatch layout;
            if (std::regex_search(line, layout, layoutLine))
            {
                const std::string qualifiers = layout[1].str();
                if (qualifiers.find("std140") != std::string::npos)
                {
                    ++std140Blocks;
                    if (qualifiers.find("binding") != std::string::npos)
                        Fail(where + ": std140 layout still sets a binding: " + line);
                }
                if (qualifiers.find("std430") != std::string::npos)
                {
                    ++std430Blocks;
                    if (!std::regex_search(qualifiers, std::regex(R"(binding\s*=\s*\d+)")))
                        Fail(where + ": std430 buffer lost its binding: " + line);
                }
            }

            // A uniform block is "uniform Name" followed by '{' on the same
            // or the next line; "uniform SWP swp;" is a struct, not a block.
            std::smatch block;
            if (!std::regex_search(line, block, blockOnLine))
                continue;
            const bool braceHere = block[2].matched;
            const bool braceNext = i + 1 < shader.lines.size() &&
                std::regex_search(shader.lines[i + 1], std::regex(R"(^\s*\{)"));
            if (!braceHere && !braceNext)
                continue;
            ++uniformBlocks;
            if (!InBindingTable(block[1].str()))
                Fail(where + ": uniform block " + block[1].str() + " is missing from kUniformBlockBindings");
        }
    }

    // Guard against a scan that silently matches nothing.
    CHECK(std140Blocks >= 9);
    // 17 before dysh.vert's BONEBLK became a buffer texture, 16 before
    // celborder.vert's four buffers did, 12 before glob's SHADOWBLK (vert and
    // frag) and ACTIVESHADOWS did, 9 before glob's LIGHTBLK, ACTIVELIGHTS and
    // TVLIGHTBLK did, 6 before glob's WARPSTATE, BONEBLK, POSEPOSBLK,
    // POSENORMALBLK and POSEWEIGHTBLK did, 1 before glob.vert's CACHEDLIGHTING
    // became a transform feedback bake read through a buffer texture.
    CHECK(std430Blocks == 0);
    CHECK(uniformBlocks == std140Blocks);
    std::printf("scanned %zu shaders: %d std140 blocks, %d std430 buffers\n",
        shaders.size(), std140Blocks, std430Blocks);
}

// ---- T3: continue inside a switch inside a loop ----------------------------

struct Token
{
    std::string text;
    int line;
};

// Splits `lines` into identifiers, numbers and single punctuation characters,
// skipping comments and preprocessor lines.
std::vector<Token> Tokenize(const std::vector<std::string>& lines)
{
    std::vector<Token> tokens;
    bool inBlockComment = false;
    bool inDirective = false;
    for (size_t i = 0; i < lines.size(); ++i)
    {
        const std::string& line = lines[i];
        const int lineNumber = static_cast<int>(i + 1);
        const size_t first = line.find_first_not_of(" \t");
        if (inDirective || (!inBlockComment && first != std::string::npos && line[first] == '#'))
        {
            inDirective = !line.empty() && line.back() == '\\';
            continue;
        }

        size_t c = 0;
        while (c < line.size())
        {
            if (inBlockComment)
            {
                const size_t end = line.find("*/", c);
                if (end == std::string::npos)
                    break;
                inBlockComment = false;
                c = end + 2;
                continue;
            }
            if (line.compare(c, 2, "//") == 0)
                break;
            if (line.compare(c, 2, "/*") == 0)
            {
                inBlockComment = true;
                c += 2;
                continue;
            }
            const unsigned char ch = static_cast<unsigned char>(line[c]);
            if (std::isspace(ch))
            {
                ++c;
                continue;
            }
            size_t end = c + 1;
            if (std::isalnum(ch) || ch == '_')
            {
                while (end < line.size() && (std::isalnum(static_cast<unsigned char>(line[end])) ||
                    line[end] == '_' || (std::isdigit(ch) && line[end] == '.')))
                    ++end;
            }
            tokens.push_back({ line.substr(c, end - c), lineNumber });
            c = end;
        }
    }
    return tokens;
}

// Returns the index of the ')' matching the '(' at `open`, or tokens.size().
size_t MatchingParen(const std::vector<Token>& tokens, size_t open)
{
    int depth = 0;
    for (size_t i = open; i < tokens.size(); ++i)
    {
        if (tokens[i].text == "(")
            ++depth;
        else if (tokens[i].text == ")" && --depth == 0)
            return i;
    }
    return tokens.size();
}

// Apple's OpenGL 4.1 driver compiles, but fails to link with no error in the
// info log, any program whose shader has a `continue` inside a switch inside a
// loop. Returns the lines of such continues: those whose nearest enclosing
// loop or switch is a switch that is itself inside a loop. `switches` counts
// the switch statements seen.
std::vector<int> FindContinueInSwitchInLoop(const std::vector<std::string>& lines, int& switches)
{
    enum class Kind { Block, Loop, Switch, If };
    struct Frame
    {
        Kind kind;
        bool braced;
    };

    const std::vector<Token> tokens = Tokenize(lines);
    std::vector<Frame> stack;
    std::vector<int> found;
    bool pending = false;
    Kind pendingKind = Kind::Block;

    // A braceless body ends with its statement, unless an else follows its if.
    const auto closeStatement = [&](size_t i)
    {
        while (!stack.empty() && !stack.back().braced)
        {
            const Kind kind = stack.back().kind;
            stack.pop_back();
            if (kind == Kind::If && i + 1 < tokens.size() && tokens[i + 1].text == "else")
                return;
        }
    };

    for (size_t i = 0; i < tokens.size(); ++i)
    {
        const std::string& text = tokens[i].text;
        if (pending && text != "{")
        {
            stack.push_back({ pendingKind, false });
            pending = false;
        }

        if (text == "for" || text == "while" || text == "switch" || text == "if")
        {
            const size_t close = MatchingParen(tokens, i + 1);
            if (close >= tokens.size())
                break;
            i = close;
            // "} while (x);" ends a do-while; its loop frame is already closed.
            if (text == "while" && close + 1 < tokens.size() && tokens[close + 1].text == ";")
            {
                ++i;
                closeStatement(i);
                continue;
            }
            pending = true;
            pendingKind = text == "switch" ? Kind::Switch : text == "if" ? Kind::If : Kind::Loop;
            switches += text == "switch" ? 1 : 0;
        }
        else if (text == "do" || text == "else")
        {
            pending = true;
            pendingKind = text == "do" ? Kind::Loop : Kind::Block;
        }
        else if (text == "{")
        {
            stack.push_back({ pending ? pendingKind : Kind::Block, true });
            pending = false;
        }
        else if (text == "}")
        {
            if (!stack.empty())
                stack.pop_back();
            closeStatement(i);
        }
        else if (text == ";")
        {
            closeStatement(i);
        }
        else if (text == "continue")
        {
            bool inSwitch = false;
            for (auto frame = stack.rbegin(); frame != stack.rend(); ++frame)
            {
                if (frame->kind == Kind::Switch)
                {
                    inSwitch = true;
                }
                else if (frame->kind == Kind::Loop)
                {
                    if (inSwitch)
                        found.push_back(tokens[i].line);
                    break;
                }
            }
        }
    }
    return found;
}

std::vector<std::string> SplitLines(const std::string& text)
{
    std::vector<std::string> lines;
    std::istringstream stream(text);
    for (std::string line; std::getline(stream, line);)
        lines.push_back(line);
    return lines;
}

std::string JoinLines(const std::vector<int>& lines)
{
    std::string text;
    for (int line : lines)
        text += (text.empty() ? "" : ",") + std::to_string(line);
    return text;
}

void TestContinueInSwitchScanner()
{
    // Guard the scanner itself so the shader check can't pass by matching nothing.
    int switches = 0;
    const auto scan = [&](const char* text) { return JoinLines(FindContinueInSwitchInLoop(SplitLines(text), switches)); };

    CHECK_STR_EQ(scan(
        "for (int i = 0; i < n; ++i)\n"
        "{\n"
        "    switch (k[i])\n"
        "    {\n"
        "        case 0:\n"
        "        if (i == 2)\n"
        "            continue; // the bug\n"
        "        c += 1.0;\n"
        "        break;\n"
        "    }\n"
        "}\n"), "7");
    CHECK_STR_EQ(scan(
        "for (int i = 0; i < n; ++i)\n"
        "    switch (k[i])\n"
        "    {\n"
        "        case 0: continue;\n"
        "    }\n"
        "do\n"
        "{\n"
        "    switch (k[0]) { default: if (b) x = 1; else continue; }\n"
        "} while (b);\n"), "4,8");
    // The same logic without a switch, with break, in comments, or with the
    // loop inside the case is fine.
    CHECK_STR_EQ(scan(
        "for (int i = 0; i < n; ++i)\n"
        "{\n"
        "    if (k[i] == 0)\n"
        "    {\n"
        "        if (i == 2)\n"
        "            continue;\n"
        "    }\n"
        "    switch (k[i]) { case 0: if (i == 2) break; c += 1.0; break; }\n"
        "    // switch (k[i]) { case 0: continue; }\n"
        "    /* switch (k[i]) { case 0: continue; } */\n"
        "}\n"
        "switch (k[0])\n"
        "{\n"
        "    case 0:\n"
        "    for (int i = 0; i < n; ++i)\n"
        "    {\n"
        "        if (i == 2)\n"
        "            continue;\n"
        "    }\n"
        "    break;\n"
        "}\n"), "");
    CHECK(switches == 5);
}

void TestNoContinueInSwitchInLoop()
{
    const std::vector<ShaderFile> shaders = LoadShaders();
    int switches = 0;
    for (const ShaderFile& shader : shaders)
    {
        for (int line : FindContinueInSwitchInLoop(shader.lines, switches))
        {
            Fail(shader.name + ":" + std::to_string(line) + ": continue inside a switch inside a loop; "
                "Apple's GL 4.1 driver fails to link such programs (no error in the info log), so use break "
                "when the switch ends the loop body: " + shader.lines[line - 1]);
        }
    }
    // Guard against a scan that silently matches nothing.
    CHECK(switches >= 9);
    std::printf("scanned %zu shaders for continue inside a switch inside a loop: %d switches\n",
        shaders.size(), switches);
}

const ShaderFile* FindShader(const std::vector<ShaderFile>& shaders, const std::string& name)
{
    for (const ShaderFile& shader : shaders)
    {
        if (shader.name == name)
            return &shader;
    }
    return nullptr;
}

bool DeclaresBufferBlock(const ShaderFile& shader, const std::string& block)
{
    const std::regex bufferBlock(R"(^\s*(layout\s*\([^)]*\)\s*)?(readonly\s+)?buffer\s+(\w+))");
    for (const std::string& line : shader.lines)
    {
        std::smatch match;
        if (std::regex_search(line, match, bufferBlock) && (block.empty() || match[3].str() == block))
            return true;
    }
    return false;
}

bool DeclaresUniform(const ShaderFile& shader, const std::string& type, const std::string& name)
{
    const std::regex declaration("^\\s*uniform\\s+" + type + "\\s+" + name + "\\s*;");
    for (const std::string& line : shader.lines)
    {
        if (std::regex_search(line, declaration))
            return true;
    }
    return false;
}

void TestDyshBonesUseBufferTexture()
{
    const std::vector<ShaderFile> shaders = LoadShaders();
    const ShaderFile* dysh = FindShader(shaders, "dysh.vert");
    const ShaderFile* glob = FindShader(shaders, "glob.vert");
    const ShaderFile* celborder = FindShader(shaders, "celborder.vert");
    CHECK(dysh != nullptr);
    CHECK(glob != nullptr);
    CHECK(celborder != nullptr);
    if (dysh == nullptr || glob == nullptr || celborder == nullptr)
        return;

    // GLSL 4.10 has no buffer blocks; dysh reads bones from a samplerBuffer.
    CHECK(!DeclaresBufferBlock(*dysh, ""));
    CHECK(DeclaresUniform(*dysh, "samplerBuffer", "boneMatrixTex"));
    // glob reads the same bone buffer texture.
    CHECK(!DeclaresBufferBlock(*glob, "BONEBLK"));
    CHECK(DeclaresUniform(*glob, "samplerBuffer", "boneMatrixTex"));
}

void TestCelBorderUsesBufferTextures()
{
    const std::vector<ShaderFile> shaders = LoadShaders();
    const ShaderFile* glob = FindShader(shaders, "glob.vert");
    const ShaderFile* celborder = FindShader(shaders, "celborder.vert");
    CHECK(glob != nullptr);
    CHECK(celborder != nullptr);
    if (glob == nullptr || celborder == nullptr)
        return;

    // EDGEBUFFER, BONEBLK, EDGEPOSEPOSBLK and POSEWEIGHTBLK became buffer textures.
    CHECK(!DeclaresBufferBlock(*celborder, ""));
    // Edge points are viewed as integers; see kEdgeTextureFormat.
    CHECK(DeclaresUniform(*celborder, "usamplerBuffer", "edgeTex"));
    CHECK(DeclaresUniform(*celborder, "samplerBuffer", "edgePoseDposTex"));
    CHECK(DeclaresUniform(*celborder, "samplerBuffer", "poseWeightTex"));
    CHECK(DeclaresUniform(*celborder, "samplerBuffer", "boneMatrixTex"));
    CHECK(kEdgeTextureFormat == GL_RGBA32UI);

    // glob.vert shares the bone and pose-weight buffer textures.
    CHECK(!DeclaresBufferBlock(*glob, "BONEBLK"));
    CHECK(!DeclaresBufferBlock(*glob, "POSEWEIGHTBLK"));
    CHECK(DeclaresUniform(*glob, "samplerBuffer", "boneMatrixTex"));
    CHECK(DeclaresUniform(*glob, "samplerBuffer", "poseWeightTex"));
}

// GL 4.1 core forward-compatible contexts reject glLineWidth above 1, so the
// Mac build widens cel outlines in a geometry stage (gl41/celborder.geom,
// which has no shared counterpart) that render.cpp feeds through
// kCelLineWidthUniform and kCelViewportUniform.
void TestCelBorderGeometryStage()
{
    const std::vector<ShaderFile> shaders = LoadShaders();
    const ShaderFile* geom = FindShader(shaders, kCelBorderGeometryShader);
    CHECK(geom != nullptr);
    if (geom == nullptr)
        return;
    CHECK(!std::filesystem::exists(std::filesystem::path(CANE_SHADER_DIR) / kCelBorderGeometryShader));

    std::string text;
    for (const std::string& line : geom->lines)
        text += line + "\n";
    CHECK(std::regex_search(text, std::regex(R"(^\s*layout\s*\(\s*lines\s*\)\s*in\s*;)",
        std::regex::multiline)));
    CHECK(std::regex_search(text, std::regex(
        R"(^\s*layout\s*\(\s*triangle_strip\s*,\s*max_vertices\s*=\s*4\s*\)\s*out\s*;)",
        std::regex::multiline)));
    CHECK(DeclaresUniform(*geom, "float", kCelLineWidthUniform));
    CHECK(DeclaresUniform(*geom, "vec2", kCelViewportUniform));
    CHECK(DeclaresUniform(*geom, "bool", kCelMultisampledUniform));
}

// Returns the lines between "// probe:begin <name>" and "// probe:end <name>",
// joined with '\n'; empty if the region is missing.
std::string ProbeRegion(const ShaderFile& shader, const std::string& name)
{
    std::string text;
    bool inside = false;
    for (const std::string& line : shader.lines)
    {
        if (line == "// probe:begin " + name)
        {
            inside = true;
            continue;
        }
        if (line == "// probe:end " + name)
            return text;
        if (inside)
            text += line + "\n";
    }
    return {};
}

void TestGlobShadowsUseBufferTextures()
{
    const std::vector<ShaderFile> shaders = LoadShaders();
    const ShaderFile* vert = FindShader(shaders, "glob.vert");
    const ShaderFile* frag = FindShader(shaders, "glob.frag");
    CHECK(vert != nullptr);
    CHECK(frag != nullptr);
    if (vert == nullptr || frag == nullptr)
        return;

    // SHADOWBLK and ACTIVESHADOWS became buffer textures; shadow records are
    // viewed as integers like celborder's edges (see kShadowTextureFormat).
    CHECK(!DeclaresBufferBlock(*vert, "SHADOWBLK"));
    CHECK(!DeclaresBufferBlock(*vert, "ACTIVESHADOWS"));
    CHECK(!DeclaresBufferBlock(*frag, "SHADOWBLK"));
    CHECK(DeclaresUniform(*vert, "usamplerBuffer", "shadowTex"));
    CHECK(DeclaresUniform(*frag, "usamplerBuffer", "shadowTex"));
    CHECK(DeclaresUniform(*vert, "isamplerBuffer", "activeShadowTex"));
    CHECK(kShadowTextureFormat == GL_RGBA32UI);
    CHECK(kActiveShadowTextureFormat == GL_R32I);

    // The record layout and its readers are shared text; shader_gl_test runs
    // both copies against the C++ layout.
    for (const char* name : { "SHADOW", "ShadowCount", "FetchShadow" })
    {
        const std::string vertRegion = ProbeRegion(*vert, name);
        const std::string fragRegion = ProbeRegion(*frag, name);
        if (vertRegion.empty() || fragRegion.empty())
            Fail(std::string("probe region ") + name + " is missing from glob.vert or glob.frag");
        CHECK_STR_EQ(vertRegion, fragRegion);
    }
    for (const char* name : { "ActiveShadowCount", "ActiveShadowIndex" })
    {
        if (ProbeRegion(*vert, name).empty())
            Fail(std::string("probe region ") + name + " is missing from glob.vert");
    }
}

void TestGlobLightsUseBufferTextures()
{
    const std::vector<ShaderFile> shaders = LoadShaders();
    const ShaderFile* vert = FindShader(shaders, "glob.vert");
    const ShaderFile* frag = FindShader(shaders, "glob.frag");
    CHECK(vert != nullptr);
    CHECK(frag != nullptr);
    if (vert == nullptr || frag == nullptr)
        return;

    // LIGHTBLK, ACTIVELIGHTS and TVLIGHTBLK became buffer textures; light
    // records mix ints and floats, so they are viewed as integers like the
    // shadow records (see kLightTextureFormat).
    CHECK(!DeclaresBufferBlock(*vert, "LIGHTBLK"));
    CHECK(!DeclaresBufferBlock(*vert, "ACTIVELIGHTS"));
    CHECK(!DeclaresBufferBlock(*vert, "TVLIGHTBLK"));
    CHECK(DeclaresUniform(*vert, "usamplerBuffer", "lightTex"));
    CHECK(DeclaresUniform(*vert, "isamplerBuffer", "activeLightTex"));
    CHECK(DeclaresUniform(*vert, "usamplerBuffer", "tvLightTex"));
    CHECK(kLightTextureFormat == GL_RGBA32UI);
    CHECK(kActiveLightTextureFormat == GL_R32I);
    CHECK(kTvLightTextureFormat == GL_RGBA32UI);

    // glob.frag never read lights.
    CHECK(!DeclaresBufferBlock(*frag, "LIGHTBLK"));
    CHECK(!DeclaresUniform(*frag, R"(\w*sampler\w*)", "lightTex"));

    // shader_gl_test runs these against the C++ upload layout.
    for (const char* name : { "LIGHT", "FetchLightAt", "FetchLight", "FetchTvLight", "LightHeaderCount",
        "AvailableLightCount", "TvLightCount", "StaticLightCount", "StaticLightIndex", "DynamicLightCount",
        "DynamicLightIndex" })
    {
        if (ProbeRegion(*vert, name).empty())
            Fail(std::string("probe region ") + name + " is missing from glob.vert");
    }
}

// The text of a function definition: its signature line through the first
// line that is just "}". Empty if `signature` is missing.
std::string FunctionText(const ShaderFile& shader, const std::string& signature)
{
    std::string text;
    bool inside = false;
    for (const std::string& line : shader.lines)
    {
        if (line == signature)
            inside = true;
        if (!inside)
            continue;
        text += line + "\n";
        if (line == "}")
            return text;
    }
    return {};
}

void TestGlobPerObjectBuffersUseBufferTextures()
{
    const std::vector<ShaderFile> shaders = LoadShaders();
    const ShaderFile* vert = FindShader(shaders, "glob.vert");
    const ShaderFile* dysh = FindShader(shaders, "dysh.vert");
    const ShaderFile* celborder = FindShader(shaders, "celborder.vert");
    CHECK(vert != nullptr);
    CHECK(dysh != nullptr);
    CHECK(celborder != nullptr);
    if (vert == nullptr || dysh == nullptr || celborder == nullptr)
        return;

    // WARPSTATE, BONEBLK, POSEPOSBLK, POSENORMALBLK, POSEWEIGHTBLK and
    // CACHEDLIGHTING became buffer textures; glob.vert has no buffer block left.
    for (const char* block : { "WARPSTATE", "BONEBLK", "POSEPOSBLK", "POSENORMALBLK", "POSEWEIGHTBLK",
        "CACHEDLIGHTING" })
    {
        if (DeclaresBufferBlock(*vert, block))
            Fail(std::string("glob.vert still declares buffer block ") + block);
    }
    CHECK(!DeclaresBufferBlock(*vert, ""));
    int std430Lines = 0;
    for (const std::string& line : vert->lines)
        std430Lines += std::regex_search(line, std::regex(R"(^\s*layout\s*\([^)]*\bstd430\b)")) ? 1 : 0;
    CHECK(std430Lines == 0);

    // All plain floats, so float views (unlike the integer views of the
    // shadow and light records).
    CHECK(DeclaresUniform(*vert, "samplerBuffer", "warpStateTex"));
    CHECK(DeclaresUniform(*vert, "samplerBuffer", "boneMatrixTex"));
    CHECK(DeclaresUniform(*vert, "samplerBuffer", "poseDposTex"));
    CHECK(DeclaresUniform(*vert, "samplerBuffer", "poseDnormalTex"));
    CHECK(DeclaresUniform(*vert, "samplerBuffer", "poseWeightTex"));
    CHECK(kWarpStateTextureFormat == GL_RGBA32F);
    CHECK(kBoneMatrixTextureFormat == GL_RGBA32F);
    CHECK(BufferSamplerFormatForUnit(kBoneMatrixTextureUnit) == kBoneMatrixTextureFormat);
    CHECK(kPoseDposTextureFormat == GL_RGBA32F);
    CHECK(kPoseDnormalTextureFormat == GL_RGBA32F);
    CHECK(kPoseWeightTextureFormat == GL_R32F);

    // No reads of the old block members are left (comments aside).
    const std::regex oldMember(R"(\b(warpState|boneMatrices|poseDpos|poseDnormal|poseWeights|cachedMaterial)\b)");
    for (size_t i = 0; i < vert->lines.size(); ++i)
    {
        const std::string code = vert->lines[i].substr(0, vert->lines[i].find("//"));
        if (std::regex_search(code, oldMember))
            Fail("glob.vert:" + std::to_string(i + 1) + " still reads a buffer block member: " + vert->lines[i]);
    }

    // The bone fetch is dysh.vert's and celborder.vert's, word for word.
    const std::string region = ProbeRegion(*vert, "BoneMatrix");
    if (region.empty())
        Fail("probe region BoneMatrix is missing from glob.vert");
    const std::string signature = "mat4 BoneMatrix(uint bone)";
    CHECK(!FunctionText(*dysh, signature).empty());
    CHECK_STR_EQ(region, FunctionText(*dysh, signature));
    CHECK_STR_EQ(region, FunctionText(*celborder, signature));
}

void TestGlobCachedLightingUsesTransformFeedback()
{
    const std::vector<ShaderFile> shaders = LoadShaders();
    const ShaderFile* vert = FindShader(shaders, "glob.vert");
    CHECK(vert != nullptr);
    if (vert == nullptr)
        return;

    // GLSL 4.10 has no buffer blocks, and no shader declares one any more.
    for (const ShaderFile& shader : shaders)
    {
        if (DeclaresBufferBlock(shader, ""))
            Fail(shader.name + " still declares a buffer block");
    }

    // The cache is baked by transform feedback and read as RGBA32F texels,
    // three per 48-byte MATERIAL record.
    CHECK(DeclaresUniform(*vert, "samplerBuffer", "cachedMaterialTex"));
    CHECK(DeclaresUniform(*vert, "int", "fCacheBake"));
    CHECK(kCachedMaterialTextureFormat == GL_RGBA32F);
    CHECK(kCachedMaterialStride == 48);
    CHECK(kGlobFeedbackVaryingCount == 4);
    CHECK_STR_EQ(kGlobFeedbackVaryings[0], "material.ambient");
    CHECK_STR_EQ(kGlobFeedbackVaryings[1], "gl_SkipComponents3");
    CHECK_STR_EQ(kGlobFeedbackVaryings[2], "material.midtone");
    CHECK_STR_EQ(kGlobFeedbackVaryings[3], "material.light");

    const std::string read = ProbeRegion(*vert, "CacheReadMaterial");
    if (read.empty())
        Fail("probe region CacheReadMaterial is missing from glob.vert");
    CHECK(read.find("textureSize(cachedMaterialTex) / 3") != std::string::npos);

    // The bake writes what CacheWriteMaterial wrote: static lights only.
    for (const std::string& line : vert->lines)
    {
        if (line.substr(0, line.find("//")).find("CacheWriteMaterial") != std::string::npos)
            Fail("glob.vert still has CacheWriteMaterial: " + line);
    }
    const std::string threeWay = FunctionText(*vert, "void StartThreeWay()");
    CHECK(!threeWay.empty());
    CHECK(threeWay.find("if (op.fDynamicLight == 1 && fCacheBake == 0)") != std::string::npos);
}

void TestSamplerUnitTable()
{
    const std::vector<ShaderFile> shaders = LoadShaders();
    int found = 0;
    for (const SamplerUnit& entry : kSamplerUnits)
    {
        const std::string name = entry.name;
        // Every entry must name a sampler some shader actually declares.
        bool declared = false;
        for (const ShaderFile& shader : shaders)
            declared = declared || DeclaresUniform(shader, R"(\w*sampler\w*)", name);
        if (!declared)
            Fail("sampler unit table entry " + name + " is not declared by any shader");

        if (name == "boneMatrixTex")
        {
            CHECK(entry.unit == kBoneMatrixTextureUnit);
            ++found;
        }
        else if (name == "edgeTex")
        {
            CHECK(entry.unit == kEdgeTextureUnit);
            ++found;
        }
        else if (name == "edgePoseDposTex")
        {
            CHECK(entry.unit == kEdgePoseDposTextureUnit);
            ++found;
        }
        else if (name == "poseWeightTex")
        {
            CHECK(entry.unit == kPoseWeightTextureUnit);
            ++found;
        }
        else if (name == "shadowTex")
        {
            CHECK(entry.unit == kShadowTextureUnit);
            ++found;
        }
        else if (name == "activeShadowTex")
        {
            CHECK(entry.unit == kActiveShadowTextureUnit);
            ++found;
        }
        else if (name == "lightTex")
        {
            CHECK(entry.unit == kLightTextureUnit);
            ++found;
        }
        else if (name == "activeLightTex")
        {
            CHECK(entry.unit == kActiveLightTextureUnit);
            ++found;
        }
        else if (name == "tvLightTex")
        {
            CHECK(entry.unit == kTvLightTextureUnit);
            ++found;
        }
        else if (name == "warpStateTex")
        {
            CHECK(entry.unit == kWarpStateTextureUnit);
            ++found;
        }
        else if (name == "poseDposTex")
        {
            CHECK(entry.unit == kPoseDposTextureUnit);
            ++found;
        }
        else if (name == "poseDnormalTex")
        {
            CHECK(entry.unit == kPoseDnormalTextureUnit);
            ++found;
        }
        else if (name == "cachedMaterialTex")
        {
            CHECK(entry.unit == kCachedMaterialTextureUnit);
            ++found;
        }
        else
        {
            Fail("unexpected sampler unit table entry " + name);
        }
    }
    CHECK(found == 13);
    CHECK(kLightTextureUnit == 6);
    CHECK(kActiveLightTextureUnit == 7);
    CHECK(kTvLightTextureUnit == 8);
    CHECK(kWarpStateTextureUnit == 9);
    CHECK(kPoseDposTextureUnit == 10);
    CHECK(kPoseDnormalTextureUnit == 11);
    // Units 0-3 hold material and shadow textures. A vertex shader may use 16
    // samplers (GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS), but unit numbers only have
    // to stay below GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS: 48 on any GL 3.2+
    // context, 80 on this Mac. Units must differ, since celborder binds four
    // entries at once and glob binds several.
    CHECK(kBoneMatrixTextureUnit == 15);
    CHECK(kCachedMaterialTextureUnit == 16);
    for (const SamplerUnit& entry : kSamplerUnits)
    {
        CHECK(entry.unit > 3 && entry.unit < 48);
        int sameUnit = 0;
        for (const SamplerUnit& other : kSamplerUnits)
            sameUnit += other.unit == entry.unit ? 1 : 0;
        CHECK(sameUnit == 1);
    }
}

void TestBindingTableMatchesStreamBindings()
{
    // gl.cpp/render.cpp bind CMGL at 0 and every per-draw stream at 1.
    int found = 0;
    for (const UniformBlockBinding& entry : kUniformBlockBindings)
    {
        const std::string name = entry.name;
        if (name == "CMGL")
        {
            CHECK(entry.binding == 0u);
            ++found;
        }
        else if (name == "RO" || name == "ROCEL" || name == "BlipGroup")
        {
            CHECK(entry.binding == 1u);
            ++found;
        }
        else
        {
            Fail("unexpected binding table entry " + name);
        }
    }
    CHECK(found == 4);
}

// ---- Drift guard: gl41 copies against the shared shaders they came from ----

std::string ReadFileBytes(const std::filesystem::path& path)
{
    std::ifstream stream(path, std::ios::binary);
    return std::string(std::istreambuf_iterator<char>(stream), std::istreambuf_iterator<char>());
}

std::string Sha256Hex(const std::string& bytes)
{
    unsigned char digest[CC_SHA256_DIGEST_LENGTH];
    CC_SHA256(bytes.data(), static_cast<CC_LONG>(bytes.size()), digest);
    static constexpr char hex[] = "0123456789abcdef";
    std::string text;
    for (unsigned char byte : digest)
    {
        text += hex[byte >> 4];
        text += hex[byte & 0xf];
    }
    return text;
}

// gl41/ORIGINS lists "<sha256>  <name>" for each Mac copy: the hash of the
// shared shader the copy was derived from. A shared shader that no longer
// matches has changed upstream and its gl41 copy must be brought up to date.
// "mac-only  <name>" marks a Mac addition with no shared counterpart; it must
// not have one, so a copy can't be relabelled to skip its hash check.
// Returns one message per problem; `verified` counts the entries that checked out.
std::vector<std::string> CheckGl41Origins(const std::filesystem::path& shaderDir, int& verified)
{
    std::vector<std::string> problems;
    verified = 0;
    const std::filesystem::path gl41Dir = shaderDir / "gl41";
    std::ifstream origins(gl41Dir / "ORIGINS");
    if (!origins)
    {
        problems.push_back("gl41/ORIGINS is missing");
        return problems;
    }

    std::map<std::string, std::string> expected;
    std::set<std::string> macOnly;
    const std::regex entryLine(R"(^([0-9a-f]{64}|mac-only)  (\S+)$)");
    std::string line;
    while (std::getline(origins, line))
    {
        if (!line.empty() && line.back() == '\r')
            line.pop_back();
        if (line.empty() || line[0] == '#')
            continue;
        std::smatch entry;
        if (!std::regex_match(line, entry, entryLine))
        {
            problems.push_back("gl41/ORIGINS has a malformed line: " + line);
            continue;
        }
        const std::string name = entry[2].str();
        if (expected.count(name) != 0 || macOnly.count(name) != 0)
            problems.push_back("gl41/ORIGINS lists " + name + " twice");
        if (entry[1].str() == "mac-only")
            macOnly.insert(name);
        else
            expected[name] = entry[1].str();
    }

    int files = 0;
    for (const auto& file : std::filesystem::directory_iterator(gl41Dir))
    {
        if (!IsShaderFile(file.path()))
            continue;
        ++files;
        const std::string name = file.path().filename().string();
        if (expected.find(name) == expected.end() && macOnly.find(name) == macOnly.end())
            problems.push_back("gl41/" + name + " has no gl41/ORIGINS entry");
    }
    if (files == 0)
        problems.push_back("gl41 holds no shaders");

    for (const std::string& name : macOnly)
    {
        if (!std::filesystem::exists(gl41Dir / name))
            problems.push_back("gl41/ORIGINS lists " + name + " but gl41/" + name + " does not exist");
        else if (std::filesystem::exists(shaderDir / name))
            problems.push_back("gl41/ORIGINS marks " + name + " mac-only but shared shader " + name +
                " exists; record its hash instead");
        else
            ++verified;
    }

    for (const auto& [name, hash] : expected)
    {
        if (!std::filesystem::exists(gl41Dir / name))
        {
            problems.push_back("gl41/ORIGINS lists " + name + " but gl41/" + name + " does not exist");
            continue;
        }
        const std::filesystem::path shared = shaderDir / name;
        if (!std::filesystem::exists(shared))
        {
            problems.push_back("gl41/ORIGINS lists " + name + " but shared shader " + name + " does not exist");
            continue;
        }
        if (Sha256Hex(ReadFileBytes(shared)) != hash)
            problems.push_back("shared shader " + name + " changed; update gl41/" + name +
                " and its hash in gl41/ORIGINS");
        else
            ++verified;
    }
    return problems;
}

void TestGl41CopiesMatchTheirOrigins()
{
    int verified = 0;
    for (const std::string& problem : CheckGl41Origins(std::filesystem::path(CANE_SHADER_DIR), verified))
        Fail(problem);
    CHECK(verified > 0);
}

void WriteFileBytes(const std::filesystem::path& path, const std::string& bytes)
{
    std::ofstream stream(path, std::ios::binary | std::ios::trunc);
    stream << bytes;
}

bool HasProblem(const std::vector<std::string>& problems, const std::string& text)
{
    for (const std::string& problem : problems)
    {
        if (problem.find(text) != std::string::npos)
            return true;
    }
    return false;
}

// The guard on a scratch tree: Mac additions pass without a hash, while a copy
// stays hash-checked even when it is marked mac-only.
void TestGl41OriginsGuardFixtures()
{
    const std::filesystem::path root = std::filesystem::temp_directory_path() /
        ("cane_gl41_origins_" + std::to_string(static_cast<long long>(::getpid())));
    std::filesystem::remove_all(root);
    std::filesystem::create_directories(root / "gl41");
    const std::string shared = "#version 430 core\nvoid main() {}\n";
    WriteFileBytes(root / "a.vert", shared);
    WriteFileBytes(root / "gl41" / "a.vert", "#version 430 core\n// 4.1 copy\nvoid main() {}\n");
    WriteFileBytes(root / "gl41" / "extra.geom", "#version 410 core\nvoid main() {}\n");

    const auto run = [&](const std::string& origins)
    {
        WriteFileBytes(root / "gl41" / "ORIGINS", origins);
        int verified = 0;
        std::vector<std::string> problems = CheckGl41Origins(root, verified);
        problems.push_back("verified=" + std::to_string(verified));
        return problems;
    };
    const std::string copyLine = Sha256Hex(shared) + "  a.vert\n";

    // A hashed copy plus a declared Mac addition: clean.
    std::vector<std::string> problems = run(copyLine + "mac-only  extra.geom\n");
    CHECK(problems.size() == 1 && problems[0] == "verified=2");
    // An undeclared gl41-only file still fails.
    CHECK(HasProblem(run(copyLine), "gl41/extra.geom has no gl41/ORIGINS entry"));
    // A copy relabelled mac-only to dodge its hash fails.
    CHECK(HasProblem(run("mac-only  a.vert\nmac-only  extra.geom\n"),
        "marks a.vert mac-only but shared shader a.vert exists"));
    // A stale copy hash still fails.
    WriteFileBytes(root / "a.vert", shared + "// upstream change\n");
    CHECK(HasProblem(run(copyLine + "mac-only  extra.geom\n"), "shared shader a.vert changed"));
    // A mac-only entry for a missing file fails, as does listing a name twice.
    CHECK(HasProblem(run(copyLine + "mac-only  extra.geom\nmac-only  gone.frag\n"),
        "lists gone.frag but gl41/gone.frag does not exist"));
    CHECK(HasProblem(run(copyLine + "mac-only  extra.geom\nmac-only  extra.geom\n"),
        "lists extra.geom twice"));

    std::filesystem::remove_all(root);
}

void TestSha256()
{
    CHECK_STR_EQ(Sha256Hex(""), "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855");
    CHECK_STR_EQ(Sha256Hex("abc"), "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad");
}
}

int main()
{
    TestRewriteReplacesLeading430Core();
    TestRewriteLeaves330Untouched();
    TestRewriteLeavesLaterVersionTextUntouched();
    TestRewriteKeepsBomAndLeadingWhitespace();
    TestRewriteHandlesCrlfAndEndOfInput();
    TestRewriteRequiresWholeDirective();
    TestShaderSources();
    TestContinueInSwitchScanner();
    TestNoContinueInSwitchInLoop();
    TestBindingTableMatchesStreamBindings();
    TestDyshBonesUseBufferTexture();
    TestCelBorderUsesBufferTextures();
    TestCelBorderGeometryStage();
    TestGlobShadowsUseBufferTextures();
    TestGlobLightsUseBufferTextures();
    TestGlobPerObjectBuffersUseBufferTextures();
    TestGlobCachedLightingUsesTransformFeedback();
    TestSamplerUnitTable();
    TestSha256();
    TestGl41CopiesMatchTheirOrigins();
    TestGl41OriginsGuardFixtures();

    if (g_failures != 0)
    {
        std::fprintf(stderr, "%d of %d checks failed\n", g_failures, g_checks);
        return 1;
    }
    std::printf("All %d checks passed\n", g_checks);
    return 0;
}
