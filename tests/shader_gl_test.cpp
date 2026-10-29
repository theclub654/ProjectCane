// Compiles and links the embedded game shaders on a hidden OpenGL 4.1
// forward-compatible core context, the most macOS offers. Shaders go through
// the same RewriteShaderVersion / ApplyUniformBlockBindings code the game uses.
// Also runs dysh.vert's skinning and celborder.vert's edges through transform
// feedback against glm references, runs glob's shadow fetch helpers against
// the C++ upload layout, runs its light fetch helpers the same way, links the
// whole unmodified glob program with the game's transform feedback varyings,
// runs that whole glob vertex shader through transform feedback, bakes and
// reads back its three-way material cache, draws blip instance ranges
// without the GL 4.2 base-instance call, draws cel outlines through
// celborder's Mac-only geometry stage and measures their width, and checks
// the glTexStorage fallback loader and the placeholder buffer textures that
// keep every buffer sampler's unit bound.
//
// Exit codes: 0 pass, 1 failure, 77 skipped (no display / no 4.1 context).

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "../Sly1-Release/glshaders.h"
#include "background_app.h"
#include "embedded_shaders.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <exception>
#include <limits>
#include <regex>
#include <sstream>
#include <string>
#include <vector>

#include <fcntl.h>
#include <unistd.h>

namespace
{
constexpr int kSkipReturnCode = 77;

int g_failures = 0;
int g_checks = 0;

#define CHECK(condition) Check((condition), #condition, __FILE__, __LINE__)

void Check(bool condition, const char* expression, const char* file, int line)
{
    ++g_checks;
    if (condition)
        return;
    ++g_failures;
    std::fprintf(stderr, "%s:%d: CHECK failed: %s\n", file, line, expression);
}

std::string FirstLine(const std::string& text)
{
    const size_t end = text.find('\n');
    return end == std::string::npos ? text : text.substr(0, end);
}

struct StageResult
{
    GLuint shader = 0;
    std::string error;
};

StageResult CompileSource(GLenum type, const char* label, const std::string& source);

StageResult CompileStage(GLenum type, const char* file)
{
    StageResult result;
    std::string source;
    try
    {
        source = RewriteShaderVersion(CaneEmbeddedShader(file), kShaderVersionGL41);
    }
    catch (const std::exception& exception)
    {
        result.error = exception.what();
        return result;
    }
    return CompileSource(type, file, source);
}

StageResult CompileSource(GLenum type, const char* file, const std::string& source)
{
    StageResult result;
    result.shader = glCreateShader(type);
    const char* text = source.c_str();
    glShaderSource(result.shader, 1, &text, nullptr);
    glCompileShader(result.shader);
    GLint compiled = GL_FALSE;
    glGetShaderiv(result.shader, GL_COMPILE_STATUS, &compiled);
    if (compiled == GL_FALSE)
    {
        GLint length = 0;
        glGetShaderiv(result.shader, GL_INFO_LOG_LENGTH, &length);
        std::string log(static_cast<size_t>(length > 1 ? length : 1), '\0');
        glGetShaderInfoLog(result.shader, length, nullptr, log.data());
        result.error = std::string(file) + ": " + FirstLine(log.c_str());
        glDeleteShader(result.shader);
        result.shader = 0;
    }
    return result;
}

// Links the given stages the way GLSHADER::Init does, then applies the
// uniform block table. Returns 0 and fills `error` when a stage failed to
// compile (the first one's message) or the link fails. `beforeLink` runs on
// the program after the stages are attached.
GLuint LinkProgramStages(const std::vector<StageResult>& stages, std::string& error,
    void (*beforeLink)(GLuint program) = nullptr)
{
    const auto failed = std::find_if(stages.begin(), stages.end(),
        [](const StageResult& stage) { return stage.shader == 0; });
    if (failed != stages.end())
    {
        error = failed->error;
        for (const StageResult& stage : stages)
        {
            if (stage.shader != 0)
                glDeleteShader(stage.shader);
        }
        return 0;
    }

    const GLuint program = glCreateProgram();
    for (const StageResult& stage : stages)
        glAttachShader(program, stage.shader);
    if (beforeLink != nullptr)
        beforeLink(program);
    glLinkProgram(program);
    for (const StageResult& stage : stages)
        glDeleteShader(stage.shader);

    GLint linked = GL_FALSE;
    glGetProgramiv(program, GL_LINK_STATUS, &linked);
    if (linked == GL_FALSE)
    {
        GLint length = 0;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &length);
        std::string log(static_cast<size_t>(length > 1 ? length : 1), '\0');
        glGetProgramInfoLog(program, length, nullptr, log.data());
        error = std::string("link: ") + FirstLine(log.c_str());
        glDeleteProgram(program);
        return 0;
    }

    ApplyUniformBlockBindings(program);
    return program;
}

// A vertex + fragment pair.
GLuint LinkStages(StageResult vertex, StageResult fragment, std::string& error,
    void (*beforeLink)(GLuint program))
{
    return LinkProgramStages({ vertex, fragment }, error, beforeLink);
}

GLuint BuildProgram(const char* vertexFile, const char* fragmentFile, std::string& error,
    void (*beforeLink)(GLuint program) = nullptr)
{
    return LinkStages(CompileStage(GL_VERTEX_SHADER, vertexFile),
        CompileStage(GL_FRAGMENT_SHADER, fragmentFile), error, beforeLink);
}

// Returns how many table blocks the program has.
int CheckBlockBindings(const char* label, GLuint program)
{
    int blocks = 0;
    for (const UniformBlockBinding& entry : kUniformBlockBindings)
    {
        const GLuint index = glGetUniformBlockIndex(program, entry.name);
        if (index == GL_INVALID_INDEX)
            continue;
        GLint binding = -1;
        glGetActiveUniformBlockiv(program, index, GL_UNIFORM_BLOCK_BINDING, &binding);
        std::printf("  %s: block %s -> binding %d (expected %u)\n",
            label, entry.name, binding, entry.binding);
        CHECK(binding == static_cast<GLint>(entry.binding));
        ++blocks;
    }
    return blocks;
}

// Returns how many table samplers the program has.
int CheckSamplerUnits(const char* label, GLuint program)
{
    int samplers = 0;
    for (const SamplerUnit& entry : kSamplerUnits)
    {
        const GLint location = glGetUniformLocation(program, entry.name);
        if (location < 0)
            continue;
        GLint unit = -1;
        glGetUniformiv(program, location, &unit);
        std::printf("  %s: sampler %s -> unit %d (expected %d)\n", label, entry.name, unit, entry.unit);
        CHECK(unit == entry.unit);
        ++samplers;
    }
    return samplers;
}

// Draws one point through `program` and returns what
// the driver wrote to stderr meanwhile. Apple's 4.1 driver reports a buffer
// sampler whose unit has no buffer texture there ("... GLD_TEXTURE_INDEX_BUFFER
// is unloadable ..."), once per process, and raises no GL error.
std::string DriverLogAroundPoint(GLuint program)
{
    char path[] = "/tmp/cane_shader_tests_stderr_XXXXXX";
    const int fd = mkstemp(path);
    if (fd < 0)
        return "mkstemp failed";
    std::fflush(stderr);
    const int saved = dup(STDERR_FILENO);
    dup2(fd, STDERR_FILENO);
    // Zeroed uniform blocks so the draw itself is valid.
    GLuint blocks = 0;
    glGenBuffers(1, &blocks);
    glBindBuffer(GL_UNIFORM_BUFFER, blocks);
    const std::vector<unsigned char> zeros(4096, 0);
    glBufferData(GL_UNIFORM_BUFFER, static_cast<GLsizeiptr>(zeros.size()), zeros.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    for (const UniformBlockBinding& entry : kUniformBlockBindings)
        glBindBufferBase(GL_UNIFORM_BUFFER, entry.binding, blocks);
    GLuint vao = 0;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glUseProgram(program);
    glDrawArrays(GL_POINTS, 0, 1);
    glFinish();
    const GLenum drawError = glGetError();
    if (drawError != GL_NO_ERROR)
        std::printf("  probe draw raised GL error 0x%04x\n", drawError);
    for (const UniformBlockBinding& entry : kUniformBlockBindings)
        glBindBufferBase(GL_UNIFORM_BUFFER, entry.binding, 0);
    glDeleteBuffers(1, &blocks);
    glUseProgram(0);
    glBindVertexArray(0);
    glDeleteVertexArrays(1, &vao);
    std::fflush(stderr);
    dup2(saved, STDERR_FILENO);
    close(saved);
    std::string text;
    char buffer[4096];
    lseek(fd, 0, SEEK_SET);
    for (ssize_t n; (n = read(fd, buffer, sizeof(buffer))) > 0;)
        text.append(buffer, static_cast<size_t>(n));
    close(fd);
    unlink(path);
    if (!text.empty())
        std::printf("  driver said: %s", text.c_str());
    return text;
}

// Float, unsigned or signed: the sampler or format class a texel fetch sees.
char SamplerClass(GLenum samplerType)
{
    switch (samplerType)
    {
        case GL_SAMPLER_BUFFER: return 'f';
        case GL_UNSIGNED_INT_SAMPLER_BUFFER: return 'u';
        case GL_INT_SAMPLER_BUFFER: return 'i';
        default: return '?';
    }
}

char FormatClass(GLenum format)
{
    switch (format)
    {
        case GL_R32F: case GL_RGBA32F: return 'f';
        case GL_R32UI: case GL_RGBA32UI: return 'u';
        case GL_R32I: case GL_RGBA32I: return 'i';
        default: return '?';
    }
}

// Every buffer sampler in the 4.1 programs has a placeholder of its own
// class, BufferTextureOrPlaceholder hands it out for texture 0, and a draw
// with the placeholders bound leaves the driver quiet where one with an
// empty unit does not. Runs before any other glob draw: the driver message
// is printed once per process.
void TestBufferSamplerPlaceholders()
{
    std::string error;
    const GLuint glob = BuildProgram("glob.vert", "glob.frag", error);
    CHECK(glob != 0);
    if (glob == 0)
        return;
    const GLuint dysh = BuildProgram("dysh.vert", "dysh.frag", error);
    const GLuint cel = BuildProgram("celborder.vert", "celborder.frag", error);
    CHECK(dysh != 0 && cel != 0);

    int samplers = 0;
    for (GLuint program : { glob, dysh, cel })
    {
        GLint count = 0;
        glGetProgramiv(program, GL_ACTIVE_UNIFORMS, &count);
        for (GLint i = 0; i < count; ++i)
        {
            char name[128];
            GLint size = 0;
            GLenum type = 0;
            glGetActiveUniform(program, static_cast<GLuint>(i), sizeof(name), nullptr, &size, &type, name);
            if (SamplerClass(type) == '?')
                continue;
            ++samplers;
            GLint unit = -1;
            glGetUniformiv(program, glGetUniformLocation(program, name), &unit);
            const GLenum format = BufferSamplerFormatForUnit(unit);
            std::printf("  buffer sampler %s unit %d placeholder 0x%04x\n", name, unit, format);
            CHECK(format != 0);
            CHECK(FormatClass(format) == SamplerClass(type));
        }
    }
    CHECK(samplers == 11 + 1 + 4);

    // The game gives glob's material and shadow-map samplers their own units
    // and textures. Do the same here so only the buffer units decide what the
    // driver says about the probe draw.
    GLuint texture2D = 0, textureArray = 0;
    const unsigned char texel[4] = { 255, 255, 255, 255 };
    glGenTextures(1, &texture2D);
    glBindTexture(GL_TEXTURE_2D, texture2D);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, texel);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glBindTexture(GL_TEXTURE_2D, 0);
    glGenTextures(1, &textureArray);
    glBindTexture(GL_TEXTURE_2D_ARRAY, textureArray);
    glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA8, 1, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, texel);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
    {
        GLint count = 0;
        glGetProgramiv(glob, GL_ACTIVE_UNIFORMS, &count);
        GLint nextUnit = 20;
        for (GLint i = 0; i < count; ++i)
        {
            char name[128];
            GLint size = 0;
            GLenum type = 0;
            glGetActiveUniform(glob, static_cast<GLuint>(i), sizeof(name), nullptr, &size, &type, name);
            if (type != GL_SAMPLER_2D && type != GL_SAMPLER_2D_ARRAY)
                continue;
            glProgramUniform1i(glob, glGetUniformLocation(glob, name), nextUnit);
            glActiveTexture(GL_TEXTURE0 + static_cast<GLenum>(nextUnit));
            if (type == GL_SAMPLER_2D)
                glBindTexture(GL_TEXTURE_2D, texture2D);
            else
                glBindTexture(GL_TEXTURE_2D_ARRAY, textureArray);
            ++nextUnit;
        }
        glActiveTexture(GL_TEXTURE0);
    }

    for (const BufferSamplerFormat& entry : kBufferSamplerFormats)
    {
        const GLuint placeholder = BufferTextureOrPlaceholder(entry.unit, 0);
        CHECK(placeholder != 0);
        CHECK(BufferTextureOrPlaceholder(entry.unit, 1234) == 1234);
        CHECK(PlaceholderBufferTexture(entry.format) == placeholder);
        glBindTexture(GL_TEXTURE_BUFFER, placeholder);
        GLint internalFormat = 0;
        glGetTexLevelParameteriv(GL_TEXTURE_BUFFER, 0, GL_TEXTURE_INTERNAL_FORMAT, &internalFormat);
        glBindTexture(GL_TEXTURE_BUFFER, 0);
        CHECK(static_cast<GLenum>(internalFormat) == entry.format);
    }
    CHECK(BufferSamplerFormatForUnit(0) == 0);

    // The 2D-array placeholder for glob's shadowTextureArray before a level
    // has a shadow map: one 1x1 layer, created once.
    const GLuint layers = PlaceholderTexture2DArray();
    CHECK(layers != 0 && PlaceholderTexture2DArray() == layers);
    glBindTexture(GL_TEXTURE_2D_ARRAY, layers);
    GLint layerWidth = 0, layerDepth = 0;
    glGetTexLevelParameteriv(GL_TEXTURE_2D_ARRAY, 0, GL_TEXTURE_WIDTH, &layerWidth);
    glGetTexLevelParameteriv(GL_TEXTURE_2D_ARRAY, 0, GL_TEXTURE_DEPTH, &layerDepth);
    glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
    CHECK(layerWidth == 1 && layerDepth == 1);

    // Every unit holds its placeholder: the driver stays quiet.
    for (const BufferSamplerFormat& entry : kBufferSamplerFormats)
    {
        glActiveTexture(GL_TEXTURE0 + static_cast<GLenum>(entry.unit));
        glBindTexture(GL_TEXTURE_BUFFER, BufferTextureOrPlaceholder(entry.unit, 0));
    }
    glActiveTexture(GL_TEXTURE0);
    const std::string quiet = DriverLogAroundPoint(glob);
    CHECK(quiet.find("unloadable") == std::string::npos);

    // Control: the same draw with the buffer units empty, as the game's
    // first glob draw left them, makes the driver complain.
    for (const BufferSamplerFormat& entry : kBufferSamplerFormats)
    {
        glActiveTexture(GL_TEXTURE0 + static_cast<GLenum>(entry.unit));
        glBindTexture(GL_TEXTURE_BUFFER, 0);
    }
    glActiveTexture(GL_TEXTURE0);
    const std::string loud = DriverLogAroundPoint(glob);
    CHECK(loud.find("unloadable") != std::string::npos);

    for (const BufferSamplerFormat& entry : kBufferSamplerFormats)
    {
        glActiveTexture(GL_TEXTURE0 + static_cast<GLenum>(entry.unit));
        glBindTexture(GL_TEXTURE_BUFFER, 0);
    }
    glActiveTexture(GL_TEXTURE0);
    DeletePlaceholderBufferTextures();
    for (GLint unit = 20; unit < 24; ++unit)
    {
        glActiveTexture(GL_TEXTURE0 + static_cast<GLenum>(unit));
        glBindTexture(GL_TEXTURE_2D, 0);
        glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
    }
    glActiveTexture(GL_TEXTURE0);
    glDeleteTextures(1, &texture2D);
    glDeleteTextures(1, &textureArray);
    CHECK(glGetError() == GL_NO_ERROR);
    glDeleteProgram(glob);
    glDeleteProgram(dysh);
    glDeleteProgram(cel);
}

void TestTexStorageFallback()
{
    std::printf("info: before fallback glTexStorage2D %s, glTexStorage3D %s\n",
        glad_glTexStorage2D != nullptr ? "loaded" : "null",
        glad_glTexStorage3D != nullptr ? "loaded" : "null");
    CHECK(LoadTexStorageFallback(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)));
    CHECK(glad_glTexStorage2D != nullptr);
    CHECK(glad_glTexStorage3D != nullptr);
    if (glad_glTexStorage2D == nullptr)
        return;

    while (glGetError() != GL_NO_ERROR) {}
    GLuint texture = 0;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, 4, 4);
    const GLenum error = glGetError();
    GLint immutable = GL_FALSE;
    glGetTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_IMMUTABLE_FORMAT, &immutable);
    std::printf("ok: glTexStorage2D 4x4 RGBA8 -> error 0x%04x, immutable %d\n", error, immutable);
    CHECK(error == GL_NO_ERROR);
    CHECK(immutable == GL_TRUE);
    glBindTexture(GL_TEXTURE_2D, 0);
    glDeleteTextures(1, &texture);
}

// ---- dysh skinning ----------------------------------------------------------

struct SkinVertex
{
    float position[3];
    GLuint boneIndices[4];
    float boneWeights[4];
};

void CaptureGlPosition(GLuint program)
{
    const char* const varyings[] = { "gl_Position" };
    glTransformFeedbackVaryings(program, 1, varyings, GL_INTERLEAVED_ATTRIBS);
}

// Mirrors dysh.vert: weights are not normalised, out-of-range bones and zero
// weights are skipped, and an all-skipped vertex stays unskinned.
glm::vec4 ReferenceDysh(const SkinVertex& vertex, bool skin, const std::vector<glm::mat4>& bones,
    const glm::mat4& model, const glm::mat4& worldToClip)
{
    glm::vec4 position(vertex.position[0], vertex.position[1], vertex.position[2], 1.0f);
    if (skin)
    {
        glm::mat4 matSkin(0.0f);
        float totalWeight = 0.0f;
        for (int i = 0; i < 4; ++i)
        {
            if (vertex.boneWeights[i] != 0.0f && vertex.boneIndices[i] < bones.size())
            {
                matSkin += bones[vertex.boneIndices[i]] * vertex.boneWeights[i];
                totalWeight += vertex.boneWeights[i];
            }
        }
        if (totalWeight > 0.0f)
            position = matSkin * position;
    }
    return worldToClip * model * position;
}

std::vector<glm::vec4> RunDysh(GLuint program, int fSkin, const std::vector<SkinVertex>& vertices)
{
    glUniform1i(glGetUniformLocation(program, "fSkin"), fSkin);

    GLuint vao = 0;
    GLuint vbo = 0;
    GLuint capture = 0;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(vertices.size() * sizeof(SkinVertex)),
        vertices.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(SkinVertex),
        reinterpret_cast<void*>(offsetof(SkinVertex, position)));
    glEnableVertexAttribArray(4);
    glVertexAttribIPointer(4, 4, GL_UNSIGNED_INT, sizeof(SkinVertex),
        reinterpret_cast<void*>(offsetof(SkinVertex, boneIndices)));
    glEnableVertexAttribArray(5);
    glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(SkinVertex),
        reinterpret_cast<void*>(offsetof(SkinVertex, boneWeights)));

    const GLsizeiptr captureSize = static_cast<GLsizeiptr>(vertices.size() * sizeof(glm::vec4));
    glGenBuffers(1, &capture);
    glBindBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, capture);
    glBufferData(GL_TRANSFORM_FEEDBACK_BUFFER, captureSize, nullptr, GL_STATIC_READ);
    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, capture);

    glEnable(GL_RASTERIZER_DISCARD);
    glBeginTransformFeedback(GL_POINTS);
    glDrawArrays(GL_POINTS, 0, static_cast<GLsizei>(vertices.size()));
    glEndTransformFeedback();
    glDisable(GL_RASTERIZER_DISCARD);

    std::vector<glm::vec4> positions(vertices.size(), glm::vec4(-999.0f));
    glBindBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, capture);
    glGetBufferSubData(GL_TRANSFORM_FEEDBACK_BUFFER, 0, captureSize, positions.data());

    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, 0);
    glBindVertexArray(0);
    glDeleteBuffers(1, &capture);
    glDeleteBuffers(1, &vbo);
    glDeleteVertexArrays(1, &vao);
    return positions;
}

bool NearlyEqual(const glm::vec4& actual, const glm::vec4& expected)
{
    for (int i = 0; i < 4; ++i)
    {
        const float tolerance = 1e-4f * std::fmax(1.0f, std::fabs(expected[i]));
        if (!(std::fabs(actual[i] - expected[i]) <= tolerance))
            return false;
    }
    return true;
}

void CheckDyshCase(const char* name, const glm::vec4& actual, const glm::vec4& expected)
{
    const bool match = NearlyEqual(actual, expected);
    std::printf("  %s %s: got (%.5f, %.5f, %.5f, %.5f) want (%.5f, %.5f, %.5f, %.5f)\n",
        match ? "ok" : "MISMATCH", name, actual.x, actual.y, actual.z, actual.w,
        expected.x, expected.y, expected.z, expected.w);
    CHECK(match);
}

void TestDyshSkinning()
{
    std::string error;
    const GLuint program = BuildProgram("dysh.vert", "dysh.frag", error, CaptureGlPosition);
    if (program == 0)
    {
        std::fprintf(stderr, "FAIL: dysh skinning program did not build: %s\n", error.c_str());
        CHECK(program != 0);
        return;
    }
    CHECK(CheckSamplerUnits("dysh skinning", program) == 1);

    // Four distinct, non-symmetric bones so a transposed or misindexed fetch shows.
    const std::vector<glm::mat4> bones =
    {
        glm::translate(glm::mat4(1.0f), glm::vec3(1.0f, -2.0f, 0.5f)) *
            glm::rotate(glm::mat4(1.0f), 0.7f, glm::normalize(glm::vec3(0.0f, 0.0f, 1.0f))),
        glm::translate(glm::mat4(1.0f), glm::vec3(-0.5f, 0.25f, 3.0f)) *
            glm::rotate(glm::mat4(1.0f), -1.1f, glm::normalize(glm::vec3(1.0f, 0.0f, 0.0f))) *
            glm::scale(glm::mat4(1.0f), glm::vec3(1.5f, 0.75f, 2.0f)),
        glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 4.0f, -1.0f)) *
            glm::rotate(glm::mat4(1.0f), 2.3f, glm::normalize(glm::vec3(1.0f, 2.0f, -1.0f))),
        glm::rotate(glm::mat4(1.0f), 0.4f, glm::normalize(glm::vec3(0.0f, 1.0f, 0.0f))) *
            glm::scale(glm::mat4(1.0f), glm::vec3(0.5f, 1.25f, 0.8f)) +
            glm::mat4(glm::vec4(0.0f), glm::vec4(0.0f), glm::vec4(0.0f), glm::vec4(0.3f, -0.7f, 1.9f, 0.0f)),
    };
    const glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(2.0f, -1.0f, 0.5f)) *
        glm::rotate(glm::mat4(1.0f), 0.35f, glm::normalize(glm::vec3(1.0f, 1.0f, 0.0f)));
    const glm::mat4 worldToClip =
        glm::perspective(glm::radians(60.0f), 4.0f / 3.0f, 0.1f, 100.0f) *
        glm::lookAt(glm::vec3(3.0f, 4.0f, 12.0f), glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

    glUseProgram(program);
    glUniformMatrix4fv(glGetUniformLocation(program, "model"), 1, GL_FALSE, &model[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(program, "matWorldToClip"), 1, GL_FALSE, &worldToClip[0][0]);

    // Built the way glob.cpp builds it: the glm::mat4 array as-is, one column per RGBA32F texel.
    GLuint boneBuffer = 0;
    GLuint boneTexture = 0;
    glGenBuffers(1, &boneBuffer);
    glBindBuffer(GL_TEXTURE_BUFFER, boneBuffer);
    glBufferData(GL_TEXTURE_BUFFER, static_cast<GLsizeiptr>(bones.size() * sizeof(glm::mat4)),
        bones.data(), GL_DYNAMIC_DRAW);
    glGenTextures(1, &boneTexture);
    glBindTexture(GL_TEXTURE_BUFFER, boneTexture);
    glTexBuffer(GL_TEXTURE_BUFFER, GL_RGBA32F, boneBuffer);
    glBindBuffer(GL_TEXTURE_BUFFER, 0);
    // Bound the way render.cpp's DrawDysh binds it.
    glActiveTexture(GL_TEXTURE0 + kBoneMatrixTextureUnit);
    glBindTexture(GL_TEXTURE_BUFFER, boneTexture);
    glActiveTexture(GL_TEXTURE0);

    const GLuint outOfRange = static_cast<GLuint>(bones.size()); // first invalid index
    const std::vector<SkinVertex> vertices =
    {
        { { 0.5f, 1.0f, -0.25f }, { 3, 1, 0, 2 }, { 0.1f, 0.2f, 0.3f, 0.4f } }, // all four weights
        { { -1.0f, 0.5f, 2.0f }, { 0, 1, 2, 3 }, { 0.0f, 0.0f, 0.0f, 0.0f } },  // zero weights
        { { 1.5f, -0.5f, 0.75f }, { 0, outOfRange, 1, 2 }, { 0.25f, 0.5f, 0.25f, 0.0f } }, // one out of range
        { { 0.25f, 2.0f, -1.0f }, { outOfRange, 0, 0, 0 }, { 1.0f, 0.0f, 0.0f, 0.0f } },   // only out of range
        { { -0.75f, -1.25f, 0.5f }, { 2, 0, 0, 0 }, { 1.0f, 0.0f, 0.0f, 0.0f } },          // single bone
    };
    static const char* const kCaseNames[] =
    {
        "all four weights", "zero weights", "one out-of-range index", "only out-of-range index", "single bone",
    };

    std::printf("dysh skinning (transform feedback vs glm):\n");
    const std::vector<glm::vec4> skinned = RunDysh(program, 1, vertices);
    for (size_t i = 0; i < vertices.size(); ++i)
        CheckDyshCase(kCaseNames[i], skinned[i], ReferenceDysh(vertices[i], true, bones, model, worldToClip));

    const std::vector<glm::vec4> unskinned = RunDysh(program, 0, vertices);
    CheckDyshCase("fSkin=0", unskinned[0], ReferenceDysh(vertices[0], false, bones, model, worldToClip));
    // The skinned cases must actually move the vertex, or they prove nothing.
    CHECK(!NearlyEqual(ReferenceDysh(vertices[0], true, bones, model, worldToClip),
        ReferenceDysh(vertices[0], false, bones, model, worldToClip)));
    CHECK(!NearlyEqual(ReferenceDysh(vertices[2], true, bones, model, worldToClip),
        ReferenceDysh(vertices[2], false, bones, model, worldToClip)));

    const GLenum glError = glGetError();
    if (glError != GL_NO_ERROR)
        std::fprintf(stderr, "dysh skinning raised GL error 0x%04x\n", glError);
    CHECK(glError == GL_NO_ERROR);

    glActiveTexture(GL_TEXTURE0 + kBoneMatrixTextureUnit);
    glBindTexture(GL_TEXTURE_BUFFER, 0);
    glActiveTexture(GL_TEXTURE0);
    glUseProgram(0);
    glDeleteTextures(1, &boneTexture);
    glDeleteBuffers(1, &boneBuffer);
    glDeleteProgram(program);
}


// ---- celborder edges --------------------------------------------------------

// Mirrors glob.h's CELPOINT, which this GL-only test can't include.
struct CelPoint
{
    glm::vec4 position;
    glm::uvec4 boneIndices;
    glm::vec4 boneWeights;
};
static_assert(sizeof(CelPoint) == 48, "CELPOINT is three RGBA32UI texels");

struct CelBorderInput
{
    std::vector<CelPoint> points;      // four per edge: E0, E1, OA, OB
    std::vector<glm::mat4> bones;
    std::vector<glm::vec4> poseDpos;   // poseCount per point
    std::vector<float> poseWeights;    // the globset's agPoses
    int poseCount = 0;
    glm::mat4 model{ 1.0f };
    glm::mat4 worldToClip{ 1.0f };
};

const glm::vec4 kCulled(2.0f, 2.0f, 2.0f, 1.0f);

// Mirrors celborder.vert's GetEdgePosition.
glm::vec3 ReferenceEdgePosition(const CelBorderInput& input, int pointIndex, bool skin, bool pose)
{
    const CelPoint& point = input.points[pointIndex];
    glm::vec4 position = point.position;

    if (pose)
    {
        const int count = std::min(std::max(input.poseCount, 0), static_cast<int>(input.poseWeights.size()));
        const int poseBase = pointIndex * input.poseCount;
        for (int ipose = 0; ipose < count; ++ipose)
        {
            const float weight = input.poseWeights[ipose];
            const int poseIndex = poseBase + ipose;
            if (weight != 0.0f && poseIndex >= 0 && poseIndex < static_cast<int>(input.poseDpos.size()))
                position += glm::vec4(glm::vec3(input.poseDpos[poseIndex]) * weight, 0.0f);
        }
    }

    if (!skin)
        return glm::vec3(position);

    const glm::vec4 localPosition(glm::vec3(position), 1.0f);
    glm::vec4 skinnedPosition(0.0f);
    float totalWeight = 0.0f;
    for (int i = 0; i < 4; ++i)
    {
        if (point.boneWeights[i] != 0.0f && point.boneIndices[i] < input.bones.size())
        {
            skinnedPosition += input.bones[point.boneIndices[i]] * localPosition * point.boneWeights[i];
            totalWeight += point.boneWeights[i];
        }
    }
    return totalWeight > 0.0f ? glm::vec3(skinnedPosition) : glm::vec3(position);
}

// Mirrors celborder.vert's main for one GL_LINES vertex.
glm::vec4 ReferenceCelBorder(const CelBorderInput& input, int vertexId, bool skin, bool pose)
{
    const int edge = vertexId >> 1;
    const bool first = (vertexId & 1) == 0;
    const int pointBase = edge * 4;
    if (pointBase < 0 || pointBase + 3 >= static_cast<int>(input.points.size()))
        return kCulled;

    const glm::mat4 mvp = input.worldToClip * input.model;
    glm::vec4 clip[4];
    for (int i = 0; i < 4; ++i)
    {
        clip[i] = mvp * glm::vec4(ReferenceEdgePosition(input, pointBase + i, skin, pose), 1.0f);
        if (clip[i].w <= 0.0f)
            return kCulled;
    }

    const glm::vec2 a = glm::vec2(clip[0]) / clip[0].w;
    const glm::vec2 b = glm::vec2(clip[1]) / clip[1].w;
    const glm::vec2 c = glm::vec2(clip[2]) / clip[2].w;
    const glm::vec2 d = glm::vec2(clip[3]) / clip[3].w;
    const auto cross2D = [](glm::vec2 u, glm::vec2 v) { return u.x * v.y - u.y * v.x; };
    const float cross0 = cross2D(b - a, c - b);
    const float cross1 = cross2D(d - b, b - a);
    if (cross0 * cross1 > 0.0f)
        return kCulled;

    return first ? clip[0] : clip[1];
}

GLuint CreateUniformBuffer(GLuint program, const char* block, GLuint binding,
    const void* data, size_t size)
{
    const GLuint index = glGetUniformBlockIndex(program, block);
    GLint blockSize = 0;
    if (index != GL_INVALID_INDEX)
        glGetActiveUniformBlockiv(program, index, GL_UNIFORM_BLOCK_DATA_SIZE, &blockSize);
    CHECK(index != GL_INVALID_INDEX);
    CHECK(static_cast<size_t>(blockSize) >= size);

    std::vector<unsigned char> bytes(std::max(static_cast<size_t>(blockSize), size), 0);
    std::memcpy(bytes.data(), data, size);
    GLuint buffer = 0;
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_UNIFORM_BUFFER, buffer);
    glBufferData(GL_UNIFORM_BUFFER, static_cast<GLsizeiptr>(bytes.size()), bytes.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    glBindBufferBase(GL_UNIFORM_BUFFER, binding, buffer);
    return buffer;
}

GLuint FindBlockBinding(const char* name)
{
    for (const UniformBlockBinding& entry : kUniformBlockBindings)
    {
        if (std::string(entry.name) == name)
            return entry.binding;
    }
    CHECK(!"block missing from kUniformBlockBindings");
    return 0;
}

// Uploads `data` the way glob.cpp does (GL_TEXTURE_BUFFER) and views it
// through the game's CreateBufferTexture.
template <typename T>
GLuint CreateBufferWithTexture(const std::vector<T>& data, GLenum internalFormat)
{
    GLuint buffer = 0;
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_TEXTURE_BUFFER, buffer);
    glBufferData(GL_TEXTURE_BUFFER, static_cast<GLsizeiptr>(data.size() * sizeof(T)), data.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_TEXTURE_BUFFER, 0);
    CHECK(CreateBufferTexture(buffer, internalFormat) != 0);
    return buffer;
}

void DeleteBufferWithTexture(GLuint buffer)
{
    DeleteBufferTexture(buffer);
    glDeleteBuffers(1, &buffer);
}

// Draws `vertexCount` GL_LINES vertices with the textures bound the way
// render.cpp's DrawCelBorder binds them, capturing gl_Position.
std::vector<glm::vec4> RunCelBorder(GLuint program, GLsizei vertexCount, bool skin, bool pose, int poseCount,
    GLuint edgeBuffer, GLuint boneTexture, GLuint poseDposBuffer, GLuint poseWeightBuffer)
{
    glUniform1i(glGetUniformLocation(program, "fSkin"), skin ? 1 : 0);
    glUniform1i(glGetUniformLocation(program, "fPose"), pose ? 1 : 0);
    glUniform1i(glGetUniformLocation(program, "poseCount"), poseCount);

    const auto bind = [](GLint unit, GLuint texture)
    {
        glActiveTexture(GL_TEXTURE0 + unit);
        glBindTexture(GL_TEXTURE_BUFFER, texture);
    };
    // Unused textures stay unbound, as in DrawCelBorder.
    bind(kEdgeTextureUnit, GetBufferTexture(edgeBuffer));
    bind(kBoneMatrixTextureUnit, skin ? boneTexture : 0);
    bind(kEdgePoseDposTextureUnit, pose ? GetBufferTexture(poseDposBuffer) : 0);
    bind(kPoseWeightTextureUnit, pose ? GetBufferTexture(poseWeightBuffer) : 0);
    glActiveTexture(GL_TEXTURE0);

    GLuint vao = 0;
    GLuint capture = 0;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    const GLsizeiptr captureSize = static_cast<GLsizeiptr>(vertexCount * sizeof(glm::vec4));
    glGenBuffers(1, &capture);
    glBindBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, capture);
    glBufferData(GL_TRANSFORM_FEEDBACK_BUFFER, captureSize, nullptr, GL_STATIC_READ);
    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, capture);

    glEnable(GL_RASTERIZER_DISCARD);
    glBeginTransformFeedback(GL_LINES);
    glDrawArrays(GL_LINES, 0, vertexCount);
    glEndTransformFeedback();
    glDisable(GL_RASTERIZER_DISCARD);

    std::vector<glm::vec4> positions(static_cast<size_t>(vertexCount), glm::vec4(-999.0f));
    glBindBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, capture);
    glGetBufferSubData(GL_TRANSFORM_FEEDBACK_BUFFER, 0, captureSize, positions.data());

    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, 0);
    glBindVertexArray(0);
    glDeleteBuffers(1, &capture);
    glDeleteVertexArrays(1, &vao);
    for (GLint unit : { kEdgeTextureUnit, kBoneMatrixTextureUnit, kEdgePoseDposTextureUnit, kPoseWeightTextureUnit })
        bind(unit, 0);
    glActiveTexture(GL_TEXTURE0);
    return positions;
}

void TestCelBorderEdges()
{
    std::string error;
    const GLuint program = BuildProgram("celborder.vert", "celborder.frag", error, CaptureGlPosition);
    if (program == 0)
    {
        std::fprintf(stderr, "FAIL: celborder edge program did not build: %s\n", error.c_str());
        CHECK(program != 0);
        return;
    }
    CHECK(CheckSamplerUnits("celborder edges", program) == 4);

    // Edge points must be read as integers: this GPU keeps float denormals, so
    // the comparisons below can't tell a float view of small bone indices apart.
    GLenum edgeTexType = GL_NONE;
    GLint uniformCount = 0;
    glGetProgramiv(program, GL_ACTIVE_UNIFORMS, &uniformCount);
    for (GLint i = 0; i < uniformCount; ++i)
    {
        char name[64] = {};
        GLint size = 0;
        GLenum type = GL_NONE;
        glGetActiveUniform(program, static_cast<GLuint>(i), sizeof(name), nullptr, &size, &type, name);
        if (std::string(name) == "edgeTex")
            edgeTexType = type;
    }
    std::printf("  edgeTex type 0x%04x (want GL_UNSIGNED_INT_SAMPLER_BUFFER 0x%04x)\n",
        edgeTexType, GL_UNSIGNED_INT_SAMPLER_BUFFER);
    CHECK(edgeTexType == GL_UNSIGNED_INT_SAMPLER_BUFFER);

    CelBorderInput input;
    input.bones =
    {
        glm::translate(glm::mat4(1.0f), glm::vec3(0.4f, -0.3f, 0.2f)) *
            glm::rotate(glm::mat4(1.0f), 0.3f, glm::normalize(glm::vec3(0.0f, 0.0f, 1.0f))),
        glm::translate(glm::mat4(1.0f), glm::vec3(-0.5f, 0.25f, -0.4f)) *
            glm::rotate(glm::mat4(1.0f), -0.5f, glm::normalize(glm::vec3(1.0f, 0.0f, 0.0f))) *
            glm::scale(glm::mat4(1.0f), glm::vec3(1.2f, 0.9f, 1.1f)),
        glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.6f, 0.3f)) *
            glm::rotate(glm::mat4(1.0f), 0.8f, glm::normalize(glm::vec3(1.0f, 2.0f, -1.0f))),
        glm::rotate(glm::mat4(1.0f), 0.4f, glm::normalize(glm::vec3(0.0f, 1.0f, 0.0f))) *
            glm::scale(glm::mat4(1.0f), glm::vec3(0.8f, 1.25f, 0.9f)),
    };
    input.model = glm::translate(glm::mat4(1.0f), glm::vec3(0.5f, -0.25f, 0.0f)) *
        glm::rotate(glm::mat4(1.0f), 0.2f, glm::normalize(glm::vec3(1.0f, 1.0f, 0.0f)));
    input.worldToClip =
        glm::perspective(glm::radians(60.0f), 4.0f / 3.0f, 0.1f, 100.0f) *
        glm::lookAt(glm::vec3(1.0f, 2.0f, 9.0f), glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

    const GLuint outOfRange = static_cast<GLuint>(input.bones.size()); // first invalid index
    const auto point = [](float x, float y, float z, glm::uvec4 bones, glm::vec4 weights)
    {
        return CelPoint{ glm::vec4(x, y, z, 1.0f), bones, weights };
    };
    // Small nonzero bone indices on purpose: read as float bits they are denormals.
    const glm::vec4 fourWeights(0.4f, 0.3f, 0.2f, 0.1f);
    const glm::vec4 oneWeight(1.0f, 0.0f, 0.0f, 0.0f);
    input.points =
    {
        // Edge 0: silhouette (OA and OB on the same side of E0-E1), four bones per point.
        point(-1.0f, 0.0f, 0.0f, { 1, 2, 3, 0 }, fourWeights),
        point(1.0f, 0.2f, 0.1f, { 2, 3, 0, 1 }, fourWeights),
        point(0.1f, 1.0f, -0.6f, { 3, 0, 1, 2 }, fourWeights),
        point(-0.2f, 0.9f, 0.7f, { 0, 1, 2, 3 }, fourWeights),
        // Edge 1: silhouette, one bone per point.
        point(-0.5f, -1.5f, 0.5f, { 2, 0, 0, 0 }, oneWeight),
        point(1.5f, -1.3f, 0.6f, { 1, 0, 0, 0 }, oneWeight),
        point(0.6f, -0.5f, -0.1f, { 3, 0, 0, 0 }, oneWeight),
        point(0.3f, -0.6f, 1.2f, { 1, 0, 0, 0 }, oneWeight),
        // Edge 2: OA and OB straddle E0-E1, so the cull test rejects it.
        point(-1.0f, 1.5f, 0.0f, { 1, 2, 3, 0 }, fourWeights),
        point(1.0f, 1.5f, 0.0f, { 2, 3, 0, 1 }, fourWeights),
        point(0.0f, 2.5f, 0.0f, { 3, 0, 1, 2 }, fourWeights),
        point(0.0f, 0.5f, 0.0f, { 0, 1, 2, 3 }, fourWeights),
        // Edge 3: out-of-range bone indices. E0 skips one, E1 skips all of them
        // (stays unskinned), OA keeps one, OB skips all.
        point(-2.0f, -1.0f, -0.5f, { 1, outOfRange, 2, 3 }, fourWeights),
        point(0.0f, -0.8f, -0.4f, { outOfRange, outOfRange + 7, 0xFFFFFFFFu, outOfRange }, fourWeights),
        point(-0.9f, 0.0f, -1.1f, { outOfRange, 2, outOfRange, outOfRange }, fourWeights),
        point(-1.2f, -0.1f, 0.2f, { outOfRange, outOfRange, outOfRange, outOfRange }, oneWeight),
    };
    const int edgeCount = static_cast<int>(input.points.size() / 4);

    input.poseCount = 2;
    input.poseWeights = { 0.75f, -0.5f, 0.3f }; // a third globset pose this glob lacks
    for (size_t i = 0; i < input.points.size(); ++i)
    {
        const float f = static_cast<float>(i);
        input.poseDpos.push_back(glm::vec4(0.05f * f - 0.3f, 0.2f - 0.03f * f, 0.1f, 0.0f));
        input.poseDpos.push_back(glm::vec4(-0.1f, 0.04f * f, 0.15f - 0.02f * f, 0.0f));
    }

    glUseProgram(program);
    struct { glm::mat4 matWorldToClip; glm::vec4 cameraPos; } cmgl{ input.worldToClip, glm::vec4(1.0f, 2.0f, 9.0f, 1.0f) };
    struct { glm::mat4 model; glm::vec4 celRgba; float uAlphaCelBorder; } rocel{ input.model, glm::vec4(0.1f), 1.0f };
    const GLuint cmglBuffer = CreateUniformBuffer(program, "CMGL", FindBlockBinding("CMGL"), &cmgl, sizeof(cmgl));
    const GLuint rocelBuffer = CreateUniformBuffer(program, "ROCEL", FindBlockBinding("ROCEL"),
        &rocel, offsetof(decltype(rocel), uAlphaCelBorder) + sizeof(float));

    // Built the way glob.cpp builds them: uploaded through GL_TEXTURE_BUFFER,
    // viewed through CreateBufferTexture with the game's formats.
    const GLuint edgeBuffer = CreateBufferWithTexture(input.points, kEdgeTextureFormat);
    const GLuint poseDposBuffer = CreateBufferWithTexture(input.poseDpos, kEdgePoseDposTextureFormat);
    // Pose weights start with one entry and are then respecified to their full
    // count, as UploadGlobsetPoseWeights does when agPoses grows; a texture
    // that missed the respecify would clamp the pose loop to one pose.
    const GLuint poseWeightBuffer = CreateBufferWithTexture(std::vector<float>{ 9.0f }, kPoseWeightTextureFormat);
    glBindBuffer(GL_TEXTURE_BUFFER, poseWeightBuffer);
    glBufferData(GL_TEXTURE_BUFFER, static_cast<GLsizeiptr>(input.poseWeights.size() * sizeof(float)),
        input.poseWeights.data(), GL_DYNAMIC_DRAW);
    glBindBuffer(GL_TEXTURE_BUFFER, 0);
    CHECK(CreateBufferTexture(poseWeightBuffer, kPoseWeightTextureFormat) == GetBufferTexture(poseWeightBuffer));
    // The bone texture is GLOBSET::boneMatrixTexture, built by hand in glob.cpp.
    GLuint boneBuffer = 0;
    GLuint boneTexture = 0;
    glGenBuffers(1, &boneBuffer);
    glBindBuffer(GL_TEXTURE_BUFFER, boneBuffer);
    glBufferData(GL_TEXTURE_BUFFER, static_cast<GLsizeiptr>(input.bones.size() * sizeof(glm::mat4)),
        input.bones.data(), GL_DYNAMIC_DRAW);
    glGenTextures(1, &boneTexture);
    glBindTexture(GL_TEXTURE_BUFFER, boneTexture);
    glTexBuffer(GL_TEXTURE_BUFFER, GL_RGBA32F, boneBuffer);
    glBindTexture(GL_TEXTURE_BUFFER, 0);
    glBindBuffer(GL_TEXTURE_BUFFER, 0);

    struct Config
    {
        const char* name;
        bool skin;
        bool pose;
    };
    static const Config kConfigs[] =
    {
        { "static", false, false },
        { "skinned", true, false },
        { "posed", false, true },
        { "skinned+posed", true, true },
    };
    static const char* const kEdgeNames[] =
    {
        "four-bone edge", "one-bone edge", "culled edge", "out-of-range bones", "past the last edge",
    };

    // One edge more than the buffer holds: the length guard must cull it.
    const GLsizei vertexCount = static_cast<GLsizei>((edgeCount + 1) * 2);
    std::printf("celborder edges (transform feedback vs glm):\n");
    for (const Config& config : kConfigs)
    {
        const std::vector<glm::vec4> captured = RunCelBorder(program, vertexCount, config.skin, config.pose,
            input.poseCount, edgeBuffer, boneTexture, poseDposBuffer, poseWeightBuffer);
        for (int vertex = 0; vertex < vertexCount; ++vertex)
        {
            const std::string name = std::string(config.name) + " " + kEdgeNames[vertex >> 1] +
                ((vertex & 1) == 0 ? " A" : " B");
            CheckDyshCase(name.c_str(), captured[vertex], ReferenceCelBorder(input, vertex, config.skin, config.pose));
        }
    }

    // The cases must exercise what they claim, or they prove nothing.
    const auto reference = [&](int vertex, bool skin, bool pose) { return ReferenceCelBorder(input, vertex, skin, pose); };
    for (const Config& config : kConfigs)
    {
        for (int vertex : { 0, 1, 2, 3, 6, 7 })
            CHECK(!NearlyEqual(reference(vertex, config.skin, config.pose), kCulled));
        CHECK(NearlyEqual(reference(4, config.skin, config.pose), kCulled));
        CHECK(NearlyEqual(reference(edgeCount * 2, config.skin, config.pose), kCulled));
    }
    CHECK(!NearlyEqual(reference(0, true, false), reference(0, false, false)));
    CHECK(!NearlyEqual(reference(0, false, true), reference(0, false, false)));
    CHECK(!NearlyEqual(reference(0, true, true), reference(0, true, false)));
    CHECK(!NearlyEqual(reference(0, true, true), reference(0, false, true)));
    CHECK(!NearlyEqual(reference(6, true, false), reference(6, false, false)));
    // Edge 3's E1 has only out-of-range bones, so skinning leaves it alone.
    CHECK(NearlyEqual(reference(7, true, false), reference(7, false, false)));

    const GLenum glError = glGetError();
    if (glError != GL_NO_ERROR)
        std::fprintf(stderr, "celborder edges raised GL error 0x%04x\n", glError);
    CHECK(glError == GL_NO_ERROR);

    glUseProgram(0);
    glBindBufferBase(GL_UNIFORM_BUFFER, FindBlockBinding("CMGL"), 0);
    glBindBufferBase(GL_UNIFORM_BUFFER, FindBlockBinding("ROCEL"), 0);
    glDeleteBuffers(1, &cmglBuffer);
    glDeleteBuffers(1, &rocelBuffer);
    DeleteBufferWithTexture(edgeBuffer);
    DeleteBufferWithTexture(poseDposBuffer);
    DeleteBufferWithTexture(poseWeightBuffer);
    CHECK(GetBufferTexture(edgeBuffer) == 0);
    glDeleteTextures(1, &boneTexture);
    glDeleteBuffers(1, &boneBuffer);
    glDeleteProgram(program);
}

// ---- celborder outline width -----------------------------------------------

// The game's celborder program on GL 4.1 (gl.cpp): vert + geometry + frag.
GLuint BuildCelBorderProgram(std::string& error)
{
    return LinkProgramStages({ CompileStage(GL_VERTEX_SHADER, "celborder.vert"),
        CompileStage(GL_GEOMETRY_SHADER, kCelBorderGeometryShader),
        CompileStage(GL_FRAGMENT_SHADER, "celborder.frag") }, error);
}

// 2:1, so a mix-up of celViewportPx's x and y scales changes the widths.
constexpr int kOutlineTargetWidth = 240;
constexpr int kOutlineTargetHeight = 120;

struct OutlineTarget
{
    GLuint fbo = 0;
    GLuint color = 0;
    GLuint depthStencil = 0;
};

// An RGBA8 + depth/stencil target like the game's scene FBO.
OutlineTarget CreateOutlineTarget()
{
    OutlineTarget target;
    glGenFramebuffers(1, &target.fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, target.fbo);
    glGenTextures(1, &target.color);
    glBindTexture(GL_TEXTURE_2D, target.color);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, kOutlineTargetWidth, kOutlineTargetHeight, 0,
        GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, target.color, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glGenRenderbuffers(1, &target.depthStencil);
    glBindRenderbuffer(GL_RENDERBUFFER, target.depthStencil);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, kOutlineTargetWidth, kOutlineTargetHeight);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, target.depthStencil);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    CHECK(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
    return target;
}

void DeleteOutlineTarget(const OutlineTarget& target)
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDeleteFramebuffers(1, &target.fbo);
    glDeleteTextures(1, &target.color);
    glDeleteRenderbuffers(1, &target.depthStencil);
}

using OutlinePixels = std::vector<std::array<unsigned char, 4>>;

// The target after a draw, row 0 at the bottom.
struct OutlineImage
{
    OutlinePixels color;
    std::vector<float> depth;
    std::vector<unsigned char> stencil;
};

// Face culling state for a draw. render.cpp draws outlines with GL_CULL_FACE
// on, back faces culled and CCW front faces, and celborder.geom's quads are
// counter-clockwise for it.
struct OutlineCull
{
    bool enabled = true;
    GLenum cullFace = GL_BACK;
    GLenum frontFace = GL_CCW;
};

// Draws `vertexCount` GL_LINES vertices of the edges in `points` (four
// CELPOINTs per edge) with the state DrawSw uses for the first cel-border
// pass and reads the target back. `clear` false draws over what the target
// already holds.
OutlineImage DrawOutline(GLuint program, const OutlineTarget& target, const std::vector<CelPoint>& points,
    const glm::mat4& worldToClip, const glm::vec4& celRgba, GLsizei vertexCount, bool clear = true,
    OutlineCull cull = {})
{
    glBindFramebuffer(GL_FRAMEBUFFER, target.fbo);
    glViewport(0, 0, kOutlineTargetWidth, kOutlineTargetHeight);
    if (clear)
    {
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClearDepth(1.0);
        glClearStencil(0);
        glStencilMask(0xFF);
        glDepthMask(GL_TRUE);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    }

    glUseProgram(program);
    struct { glm::mat4 matWorldToClip; glm::vec4 cameraPos; } cmgl{ worldToClip, glm::vec4(0.0f, 0.0f, 0.0f, 1.0f) };
    struct { glm::mat4 model; glm::vec4 celRgba; float uAlphaCelBorder; } rocel{ glm::mat4(1.0f), celRgba, 1.0f };
    const GLuint cmglBuffer = CreateUniformBuffer(program, "CMGL", FindBlockBinding("CMGL"), &cmgl, sizeof(cmgl));
    const GLuint rocelBuffer = CreateUniformBuffer(program, "ROCEL", FindBlockBinding("ROCEL"),
        &rocel, offsetof(decltype(rocel), uAlphaCelBorder) + sizeof(float));
    const GLuint edgeBuffer = CreateBufferWithTexture(points, kEdgeTextureFormat);

    glUniform1i(glGetUniformLocation(program, "fSkin"), 0);
    glUniform1i(glGetUniformLocation(program, "fPose"), 0);
    glUniform1i(glGetUniformLocation(program, "poseCount"), 0);
    glActiveTexture(GL_TEXTURE0 + kEdgeTextureUnit);
    glBindTexture(GL_TEXTURE_BUFFER, GetBufferTexture(edgeBuffer));
    glActiveTexture(GL_TEXTURE0);

    // render.cpp: GL_CULL_FACE stays on, then the cel-border pass.
    if (cull.enabled)
        glEnable(GL_CULL_FACE);
    glCullFace(cull.cullFace);
    glFrontFace(cull.frontFace);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);
    glEnable(GL_STENCIL_TEST);
    glStencilMask(0x40);
    glStencilFunc(GL_ALWAYS, 0x40, 0x40);
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
    glDepthFunc(GL_LEQUAL);
    glDepthMask(GL_TRUE);

    GLuint vao = 0;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glDrawArrays(GL_LINES, 0, vertexCount);
    glBindVertexArray(0);
    glDeleteVertexArrays(1, &vao);

    glStencilMask(0xFF);
    glDisable(GL_STENCIL_TEST);
    glDisable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
    glDepthFunc(GL_LESS);

    const size_t count = static_cast<size_t>(kOutlineTargetWidth * kOutlineTargetHeight);
    OutlineImage image;
    image.color.resize(count);
    image.depth.resize(count);
    image.stencil.resize(count);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glReadPixels(0, 0, kOutlineTargetWidth, kOutlineTargetHeight, GL_RGBA, GL_UNSIGNED_BYTE, image.color.data());
    glReadPixels(0, 0, kOutlineTargetWidth, kOutlineTargetHeight, GL_DEPTH_COMPONENT, GL_FLOAT, image.depth.data());
    glReadPixels(0, 0, kOutlineTargetWidth, kOutlineTargetHeight, GL_STENCIL_INDEX, GL_UNSIGNED_BYTE, image.stencil.data());

    glActiveTexture(GL_TEXTURE0 + kEdgeTextureUnit);
    glBindTexture(GL_TEXTURE_BUFFER, 0);
    glActiveTexture(GL_TEXTURE0);
    glUseProgram(0);
    glBindBufferBase(GL_UNIFORM_BUFFER, FindBlockBinding("CMGL"), 0);
    glBindBufferBase(GL_UNIFORM_BUFFER, FindBlockBinding("ROCEL"), 0);
    glDeleteBuffers(1, &cmglBuffer);
    glDeleteBuffers(1, &rocelBuffer);
    DeleteBufferWithTexture(edgeBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return image;
}

bool IsBackground(const std::array<unsigned char, 4>& pixel)
{
    return pixel[0] == 0 && pixel[1] == 0 && pixel[2] == 0 && pixel[3] == 0;
}

bool MatchesColor(const std::array<unsigned char, 4>& pixel, const glm::vec4& color)
{
    for (int i = 0; i < 4; ++i)
    {
        if (std::abs(static_cast<int>(pixel[i]) - static_cast<int>(std::lround(color[i] * 255.0f))) > 1)
            return false;
    }
    return true;
}

int CountCovered(const OutlinePixels& pixels)
{
    return static_cast<int>(std::count_if(pixels.begin(), pixels.end(),
        [](const std::array<unsigned char, 4>& pixel) { return !IsBackground(pixel); }));
}

// Pixels where colour, depth and stencil disagree about being covered: a
// covered pixel has the cel-border stencil bit (0x40) and a depth below the
// clear value, an uncovered one neither. `depth`, when not negative, is the
// depth every covered pixel should have.
int CountDepthStencilMismatches(const OutlineImage& image, float depth = -1.0f)
{
    int mismatches = 0;
    for (size_t i = 0; i < image.color.size(); ++i)
    {
        const bool covered = !IsBackground(image.color[i]);
        const bool ok = covered
            ? image.stencil[i] == 0x40 && image.depth[i] < 1.0f &&
                (depth < 0.0f || std::fabs(image.depth[i] - depth) <= 1e-3f)
            : image.stencil[i] == 0 && image.depth[i] == 1.0f;
        if (!ok)
            ++mismatches;
    }
    return mismatches;
}

// GL's non-multisampled wide-line width: the requested width rounded to whole
// pixels, at least one.
int WideLineWidth(float widthPx)
{
    return (std::max)(1, static_cast<int>(std::floor(widthPx + 0.5f)));
}

struct OutlineMeasure
{
    int covered = 0;        // pixels that aren't background
    int wrongColor = 0;     // covered pixels that aren't celRgba
    float thickness[3] = {};// across the edge around 1/4, 1/2 and 3/4 of its length
    float minorSpan = 0.0f; // covered pixels per minor-axis column (row), averaged over the middle half
    int minorSpanMin = 0;
    int minorSpanMax = 0;
    float tMin = 0.0f;      // covered pixel centres along the edge, from A
    float tMax = 0.0f;
    float sMax = 0.0f;      // furthest covered pixel centre from the edge's line
};

// `a` and `b` are the edge's ends in window pixels.
OutlineMeasure MeasureOutline(const OutlinePixels& pixels, const glm::vec4& color, glm::vec2 a, glm::vec2 b)
{
    OutlineMeasure measure;
    const float length = glm::length(b - a);
    const glm::vec2 direction = length > 0.0f ? (b - a) / length : glm::vec2(1.0f, 0.0f);
    const glm::vec2 normal(-direction.y, direction.x);
    const auto coveredAt = [&](int x, int y)
    {
        if (x < 0 || y < 0 || x >= kOutlineTargetWidth || y >= kOutlineTargetHeight)
            return false;
        return !IsBackground(pixels[static_cast<size_t>(y * kOutlineTargetWidth + x)]);
    };
    const auto covered = [&](glm::vec2 p)
    {
        return coveredAt(static_cast<int>(std::floor(p.x)), static_cast<int>(std::floor(p.y)));
    };

    measure.tMin = 1e9f;
    measure.tMax = -1e9f;
    for (int y = 0; y < kOutlineTargetHeight; ++y)
    {
        for (int x = 0; x < kOutlineTargetWidth; ++x)
        {
            const auto& pixel = pixels[static_cast<size_t>(y * kOutlineTargetWidth + x)];
            if (IsBackground(pixel))
                continue;
            ++measure.covered;
            if (!MatchesColor(pixel, color))
                ++measure.wrongColor;
            const glm::vec2 centre(x + 0.5f, y + 0.5f);
            measure.tMin = std::min(measure.tMin, glm::dot(centre - a, direction));
            measure.tMax = std::max(measure.tMax, glm::dot(centre - a, direction));
            measure.sMax = std::max(measure.sMax, std::fabs(glm::dot(centre - a, normal)));
        }
    }

    // Covered pixels per unit length in a 10 px long slab across the edge:
    // the band's width perpendicular to the edge there, without the stair
    // steps a single scan line picks up on a diagonal.
    constexpr float kHalfSlab = 5.0f;
    const float fractions[3] = { 0.25f, 0.5f, 0.75f };
    for (int i = 0; i < 3; ++i)
    {
        const float t0 = length * fractions[i];
        int hits = 0;
        for (int y = 0; y < kOutlineTargetHeight; ++y)
        {
            for (int x = 0; x < kOutlineTargetWidth; ++x)
            {
                const glm::vec2 centre(x + 0.5f, y + 0.5f);
                if (covered(centre) && std::fabs(glm::dot(centre - a, direction) - t0) < kHalfSlab)
                    ++hits;
            }
        }
        measure.thickness[i] = static_cast<float>(hits) / (2.0f * kHalfSlab);
    }

    // What GL's wide lines fix: every column of an x-major edge (every row
    // of a y-major one) holds the same whole number of pixels.
    const bool xMajor = std::fabs(b.x - a.x) >= std::fabs(b.y - a.y);
    const float from = xMajor ? std::min(a.x, b.x) : std::min(a.y, b.y);
    const float span = xMajor ? std::fabs(b.x - a.x) : std::fabs(b.y - a.y);
    const int first = static_cast<int>(std::floor(from + 0.25f * span));
    const int last = static_cast<int>(std::floor(from + 0.75f * span));
    const int across = xMajor ? kOutlineTargetHeight : kOutlineTargetWidth;
    int lines = 0;
    int total = 0;
    measure.minorSpanMin = 1 << 30;
    measure.minorSpanMax = 0;
    for (int line = first; line <= last; ++line)
    {
        int count = 0;
        for (int i = 0; i < across; ++i)
            count += (xMajor ? coveredAt(line, i) : coveredAt(i, line)) ? 1 : 0;
        total += count;
        ++lines;
        measure.minorSpanMin = std::min(measure.minorSpanMin, count);
        measure.minorSpanMax = std::max(measure.minorSpanMax, count);
    }
    measure.minorSpan = lines > 0 ? static_cast<float>(total) / static_cast<float>(lines) : 0.0f;
    return measure;
}

// World point whose clip position under the identity matrix lands on window
// pixel `p` (w = 1, z = `z`).
glm::vec3 PixelToNdc(glm::vec2 p, float z = 0.0f)
{
    return glm::vec3(2.0f * p.x / kOutlineTargetWidth - 1.0f, 2.0f * p.y / kOutlineTargetHeight - 1.0f, z);
}

glm::vec2 ClipToPixel(const glm::vec4& clip)
{
    const glm::vec2 ndc = glm::vec2(clip) / clip.w;
    return (ndc * 0.5f + 0.5f) * glm::vec2(kOutlineTargetWidth, kOutlineTargetHeight);
}

// One silhouette edge E0-E1 with OA and OB on the same side, as world points.
std::vector<CelPoint> SilhouetteEdge(glm::vec3 e0, glm::vec3 e1, glm::vec3 side)
{
    const glm::vec3 middle = 0.5f * (e0 + e1);
    const auto point = [](glm::vec3 p) { return CelPoint{ glm::vec4(p, 1.0f), glm::uvec4(0), glm::vec4(0.0f) }; };
    return { point(e0), point(e1), point(middle + side * 0.5f), point(middle + side) };
}

void TestCelBorderOutlineWidth()
{
    std::string error;
    const GLuint program = BuildCelBorderProgram(error);
    if (program == 0)
    {
        std::fprintf(stderr, "FAIL: celborder vert+geom+frag program did not build: %s\n", error.c_str());
        CHECK(program != 0);
        return;
    }
    std::printf("ok: celborder vert+geom+frag compiled and linked\n");
    CHECK(CheckBlockBindings("celborder+geom", program) == 2);
    CHECK(CheckSamplerUnits("celborder+geom", program) == 4);
    const GLint widthLocation = glGetUniformLocation(program, kCelLineWidthUniform);
    const GLint viewportLocation = glGetUniformLocation(program, kCelViewportUniform);
    const GLint multisampledLocation = glGetUniformLocation(program, kCelMultisampledUniform);
    CHECK(widthLocation >= 0);
    CHECK(viewportLocation >= 0);
    CHECK(multisampledLocation >= 0);

    while (glGetError() != GL_NO_ERROR) {}
    const OutlineTarget target = CreateOutlineTarget();
    const glm::vec4 celRgba(1.0f, 0.5f, 0.25f, 1.0f);
    const glm::mat4 identity(1.0f);
    const glm::mat4 perspective =
        glm::perspective(glm::radians(60.0f), static_cast<float>(kOutlineTargetWidth) / kOutlineTargetHeight, 0.1f, 100.0f) *
        glm::lookAt(glm::vec3(0.0f, 0.0f, 5.0f), glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

    // Windows draws the outlines as GL_LINES at glLineWidth(4.125 x scale)
    // without multisampling (the default): GL rounds the width to whole
    // pixels and spreads it along the minor axis, so a diagonal is thinner
    // across than an axis-aligned edge (by cos of its angle to the major
    // axis), and its ends are square to the major axis, not to the edge.
    struct Case
    {
        const char* name;
        float width;
        std::vector<CelPoint> points;
        glm::mat4 worldToClip;
    };
    std::vector<Case> cases =
    {
        { "horizontal 4.125 px", 4.125f, SilhouetteEdge(PixelToNdc({ 30.0f, 60.25f }), PixelToNdc({ 130.0f, 60.25f }), { 0.0f, 0.5f, 0.0f }), identity },
        { "horizontal 8 px", 8.0f, SilhouetteEdge(PixelToNdc({ 30.0f, 60.0f }), PixelToNdc({ 130.0f, 60.0f }), { 0.0f, -0.5f, 0.0f }), identity },
        { "horizontal right to left 8 px", 8.0f, SilhouetteEdge(PixelToNdc({ 200.0f, 40.0f }), PixelToNdc({ 60.0f, 40.0f }), { 0.0f, 0.5f, 0.0f }), identity },
        { "vertical 4.125 px", 4.125f, SilhouetteEdge(PixelToNdc({ 80.25f, 20.0f }), PixelToNdc({ 80.25f, 100.0f }), { 0.5f, 0.0f, 0.0f }), identity },
        { "vertical 8 px", 8.0f, SilhouetteEdge(PixelToNdc({ 80.0f, 100.0f }), PixelToNdc({ 80.0f, 20.0f }), { -0.5f, 0.0f, 0.0f }), identity },
        // x-major, a little under 45 degrees, in each quadrant.
        { "diagonal up-right 4.125 px", 4.125f, SilhouetteEdge(PixelToNdc({ 30.0f, 20.0f }), PixelToNdc({ 120.0f, 100.0f }), { -0.5f, 0.5f, 0.0f }), identity },
        { "diagonal up-left 8 px", 8.0f, SilhouetteEdge(PixelToNdc({ 130.0f, 25.0f }), PixelToNdc({ 35.0f, 95.0f }), { 0.5f, 0.5f, 0.0f }), identity },
        { "diagonal down-left 4.125 px", 4.125f, SilhouetteEdge(PixelToNdc({ 210.0f, 100.0f }), PixelToNdc({ 120.0f, 20.0f }), { 0.5f, -0.5f, 0.0f }), identity },
        { "diagonal down-right 8 px", 8.0f, SilhouetteEdge(PixelToNdc({ 135.0f, 95.0f }), PixelToNdc({ 230.0f, 25.0f }), { -0.5f, -0.5f, 0.0f }), identity },
        // Exactly 45 degrees counts as x-major, as in GL.
        { "45 degrees 8 px", 8.0f, SilhouetteEdge(PixelToNdc({ 40.0f, 100.0f }), PixelToNdc({ 120.0f, 20.0f }), { 0.5f, 0.5f, 0.0f }), identity },
        // y-major diagonals, both ways.
        { "steep up-right 8 px", 8.0f, SilhouetteEdge(PixelToNdc({ 150.0f, 15.0f }), PixelToNdc({ 180.0f, 105.0f }), { -0.5f, 0.2f, 0.0f }), identity },
        { "steep down-left 4.125 px", 4.125f, SilhouetteEdge(PixelToNdc({ 100.0f, 105.0f }), PixelToNdc({ 70.0f, 15.0f }), { 0.5f, -0.2f, 0.0f }), identity },
        // clip w = 2 everywhere: the offset has to be scaled by w.
        { "w = 2 horizontal 8 px", 8.0f, SilhouetteEdge(PixelToNdc({ 30.0f, 60.0f }), PixelToNdc({ 130.0f, 60.0f }), { 0.0f, 0.5f, 0.0f }), glm::mat4(2.0f) },
        // Ends at different depths (w about 3.5 and 7), so the width must hold along the edge.
        { "perspective diagonal 8 px", 8.0f, SilhouetteEdge({ -1.4f, -1.0f, 1.5f }, { 2.5f, 1.6f, -2.0f }, { -0.3f, 0.6f, 0.0f }), perspective },
        // Rounding: unrounded, these bands would hold 4, 2 and 0 pixels per column (row).
        { "horizontal 4.6 px rounds to 5", 4.6f, SilhouetteEdge(PixelToNdc({ 30.0f, 60.9f }), PixelToNdc({ 130.0f, 60.9f }), { 0.0f, 0.5f, 0.0f }), identity },
        { "vertical 2.6 px rounds to 3", 2.6f, SilhouetteEdge(PixelToNdc({ 80.1f, 20.0f }), PixelToNdc({ 80.1f, 100.0f }), { 0.5f, 0.0f, 0.0f }), identity },
        { "horizontal 0.3 px rounds to 1", 0.3f, SilhouetteEdge(PixelToNdc({ 30.0f, 60.9f }), PixelToNdc({ 130.0f, 60.9f }), { 0.0f, 0.5f, 0.0f }), identity },
        // The game's width at a 1.5x internal resolution: 6.1875 px draws 6.
        { "diagonal 6.1875 px (1.5x)", 4.125f * 1.5f, SilhouetteEdge(PixelToNdc({ 30.0f, 30.0f }), PixelToNdc({ 150.0f, 110.0f }), { -0.5f, 0.5f, 0.0f }), identity },
    };

    glLineWidth(1.0f);
    // Without MSAA, as render.cpp sets it when the scene target isn't multisampled.
    glProgramUniform1i(program, multisampledLocation, 0);
    for (const Case& c : cases)
    {
        CelBorderInput input;
        input.points = c.points;
        input.worldToClip = c.worldToClip;
        const glm::vec4 clipA = ReferenceCelBorder(input, 0, false, false);
        const glm::vec4 clipB = ReferenceCelBorder(input, 1, false, false);
        CHECK(!NearlyEqual(clipA, kCulled));
        const glm::vec2 a = ClipToPixel(clipA);
        const glm::vec2 b = ClipToPixel(clipB);
        const float length = glm::length(b - a);
        const float major = std::max(std::fabs(b.x - a.x), std::fabs(b.y - a.y));
        const float minor = std::min(std::fabs(b.x - a.x), std::fabs(b.y - a.y));
        const int wideWidth = WideLineWidth(c.width);
        // Across the edge, and how far the minor-axis ends reach past A and B along it.
        const float thicknessWant = static_cast<float>(wideWidth) * major / length;
        const float endReach = 0.5f * static_cast<float>(wideWidth) * minor / length;

        SetCelBorderLineWidth(program, widthLocation, viewportLocation, c.width,
            kOutlineTargetWidth, kOutlineTargetHeight);
        const OutlineImage image = DrawOutline(program, target, c.points, c.worldToClip, celRgba, 2);
        const OutlineMeasure m = MeasureOutline(image.color, celRgba, a, b);
        const int depthStencilMismatches = CountDepthStencilMismatches(image,
            c.worldToClip == identity ? 0.5f : -1.0f);

        bool ok = m.wrongColor == 0 && m.covered > 0 && depthStencilMismatches == 0 &&
            m.minorSpanMin == wideWidth && m.minorSpanMax == wideWidth &&
            std::fabs(m.tMin) <= endReach + 1.0f && std::fabs(m.tMax - length) <= endReach + 1.0f &&
            m.sMax <= 0.5f * thicknessWant + 1.0f;
        for (float thickness : m.thickness)
            ok = ok && std::fabs(thickness - thicknessWant) <= 0.75f;
        std::printf("  %s %s: A (%.1f, %.1f) B (%.1f, %.1f) w %.2f/%.2f, minor-axis span %d..%d (want %d), "
            "thickness %.2f %.2f %.2f (want %.2f), along [%.2f, %.2f] of %.2f (+-%.2f), across <= %.2f, %d px, "
            "%d off-colour, %d depth/stencil mismatches\n",
            ok ? "ok" : "MISMATCH", c.name, a.x, a.y, b.x, b.y, clipA.w, clipB.w,
            m.minorSpanMin, m.minorSpanMax, wideWidth, m.thickness[0], m.thickness[1], m.thickness[2],
            thicknessWant, m.tMin, m.tMax, length, endReach + 1.0f, m.sMax, m.covered, m.wrongColor,
            depthStencilMismatches);
        CHECK(ok);
    }

    // With MSAA, GL draws a wide line as a rectangle perpendicular to it at the
    // unrounded width that stops at its ends; render.cpp sets celMultisampled
    // while the scene target is multisampled. So a 45-degree edge at the game's
    // 4.125 px is about 4.1 px across with it, and 3 px (2.83) without it.
    {
        struct MsaaCase
        {
            const char* name;
            bool multisampled;
            float width;
            std::vector<CelPoint> points;
            float thicknessWant;
        };
        // Half a pixel off the lattice: this target samples pixel centres only, so
        // a 45-degree band on it covers 5 or 6 diagonals of centres (3.5 or 4.2
        // px across) depending on where it sits.
        const std::vector<CelPoint> diagonal = SilhouetteEdge(PixelToNdc({ 40.5f, 100.0f }), PixelToNdc({ 120.5f, 20.0f }), { 0.5f, 0.5f, 0.0f });
        const MsaaCase msaaCases[] =
        {
            { "MSAA 45 degrees 4.125 px", true, 4.125f, diagonal, 4.125f },
            { "no MSAA 45 degrees 4.125 px", false, 4.125f, diagonal, 4.0f * 0.5f * std::sqrt(2.0f) },
            { "MSAA steep 4.125 px", true, 4.125f, SilhouetteEdge(PixelToNdc({ 100.0f, 105.0f }), PixelToNdc({ 50.0f, 15.0f }), { 0.5f, -0.2f, 0.0f }), 4.125f },
        };
        for (const MsaaCase& c : msaaCases)
        {
            CelBorderInput input;
            input.points = c.points;
            const glm::vec2 a = ClipToPixel(ReferenceCelBorder(input, 0, false, false));
            const glm::vec2 b = ClipToPixel(ReferenceCelBorder(input, 1, false, false));
            const float length = glm::length(b - a);
            SetCelBorderLineWidth(program, widthLocation, viewportLocation, c.width,
                kOutlineTargetWidth, kOutlineTargetHeight);
            glProgramUniform1i(program, multisampledLocation, c.multisampled ? 1 : 0);
            const OutlineMeasure m = MeasureOutline(DrawOutline(program, target, c.points, identity, celRgba, 2).color, celRgba, a, b);
            bool ok = m.wrongColor == 0 && m.covered > 0;
            for (float thickness : m.thickness)
                ok = ok && std::fabs(thickness - c.thicknessWant) <= 0.5f;
            // The rectangle stops at the ends; the minor-axis band reaches past them.
            if (c.multisampled)
                ok = ok && std::fabs(m.tMin) <= 1.0f && std::fabs(m.tMax - length) <= 1.0f &&
                    m.sMax <= 0.5f * c.width + 1.0f;
            std::printf("  %s %s: thickness %.2f %.2f %.2f (want %.2f), along [%.2f, %.2f] of %.2f, across <= %.2f, "
                "%d px, %d off-colour\n", ok ? "ok" : "MISMATCH", c.name, m.thickness[0], m.thickness[1],
                m.thickness[2], c.thicknessWant, m.tMin, m.tMax, length, m.sMax, m.covered, m.wrongColor);
            CHECK(ok);
        }
        glProgramUniform1i(program, multisampledLocation, 0);
    }

    // Edges that draw nothing.
    {
        const auto drawsNothing = [&](const char* name, const std::vector<CelPoint>& points, GLsizei vertexCount)
        {
            SetCelBorderLineWidth(program, widthLocation, viewportLocation, 8.0f, kOutlineTargetWidth, kOutlineTargetHeight);
            while (glGetError() != GL_NO_ERROR) {}
            const OutlineImage image = DrawOutline(program, target, points, identity, celRgba, vertexCount);
            const GLenum drawError = glGetError();
            const int covered = CountCovered(image.color);
            const int mismatches = CountDepthStencilMismatches(image);
            const bool ok = covered == 0 && mismatches == 0 && drawError == GL_NO_ERROR;
            std::printf("  %s %s: %d px, %d depth/stencil mismatches, GL error 0x%04x\n",
                ok ? "ok" : "MISMATCH", name, covered, mismatches, drawError);
            CHECK(ok);
        };

        // OA and OB straddle E0-E1: celborder.vert culls the edge.
        std::vector<CelPoint> culled = SilhouetteEdge(PixelToNdc({ 30.0f, 60.0f }), PixelToNdc({ 130.0f, 60.0f }), { 0.0f, 0.5f, 0.0f });
        culled[3].position = glm::vec4(PixelToNdc({ 80.0f, 20.0f }), 1.0f);
        CelBorderInput input;
        input.points = culled;
        CHECK(NearlyEqual(ReferenceCelBorder(input, 0, false, false), kCulled));
        drawsNothing("culled edge", culled, 2);
        // Vertices 2 and 3 have no edge behind them; vertex 0-1 is the culled one.
        drawsNothing("past the last edge", culled, 4);

        // An end the vertex stage put exactly on CullVertex()'s position is
        // dropped by the geometry stage's sentinel check (it lies outside the
        // clip volume, so nothing on screen is lost).
        const std::vector<CelPoint> sentinel = SilhouetteEdge(glm::vec3(kCulled), PixelToNdc({ 60.0f, 60.0f }), { 0.0f, 0.5f, 0.0f });
        input.points = sentinel;
        CHECK(NearlyEqual(ReferenceCelBorder(input, 0, false, false), kCulled));
        drawsNothing("end on the cull sentinel", sentinel, 2);
        // Control: the same edge with that end a little off the sentinel draws
        // its part inside the clip volume.
        {
            const std::vector<CelPoint> nearSentinel = SilhouetteEdge({ 2.0f, 2.0f, 1.9f }, PixelToNdc({ 60.0f, 60.0f }), { 0.0f, 0.5f, 0.0f });
            const int covered = CountCovered(DrawOutline(program, target, nearSentinel, identity, celRgba, 2).color);
            std::printf("  %s end just off the cull sentinel: %d px\n", covered > 0 ? "ok" : "MISMATCH", covered);
            CHECK(covered > 0);
        }

        // NaN ends (a degenerate skinned or posed point) draw nothing and raise no error.
        const float nan = std::numeric_limits<float>::quiet_NaN();
        std::vector<CelPoint> nanStart = SilhouetteEdge(PixelToNdc({ 30.0f, 60.0f }), PixelToNdc({ 130.0f, 60.0f }), { 0.0f, 0.5f, 0.0f });
        nanStart[0].position = glm::vec4(nan, nan, nan, 1.0f);
        drawsNothing("NaN start", nanStart, 2);
        std::vector<CelPoint> nanEnd = SilhouetteEdge(PixelToNdc({ 30.0f, 60.0f }), PixelToNdc({ 130.0f, 60.0f }), { 0.0f, 0.5f, 0.0f });
        nanEnd[1].position = glm::vec4(0.0f, nan, 0.0f, 1.0f);
        drawsNothing("NaN end", nanEnd, 2);
    }

    // The depth test applies: an outline behind one already drawn leaves it
    // alone, one in front replaces it.
    {
        const glm::vec4 behindRgba(0.25f, 1.0f, 0.5f, 1.0f);
        const auto edgeAt = [](float z)
        {
            return SilhouetteEdge(PixelToNdc({ 30.0f, 60.0f }, z), PixelToNdc({ 130.0f, 60.0f }, z), { 0.0f, 0.5f, 0.0f });
        };
        SetCelBorderLineWidth(program, widthLocation, viewportLocation, 8.0f, kOutlineTargetWidth, kOutlineTargetHeight);
        const int first = CountCovered(DrawOutline(program, target, edgeAt(0.0f), identity, celRgba, 2).color);
        const OutlineImage behind = DrawOutline(program, target, edgeAt(0.5f), identity, behindRgba, 2, false);
        const OutlineMeasure kept = MeasureOutline(behind.color, celRgba, { 30.0f, 60.0f }, { 130.0f, 60.0f });
        const OutlineImage front = DrawOutline(program, target, edgeAt(-0.5f), identity, behindRgba, 2, false);
        const OutlineMeasure replaced = MeasureOutline(front.color, behindRgba, { 30.0f, 60.0f }, { 130.0f, 60.0f });
        const bool ok = first > 0 && kept.covered == first && kept.wrongColor == 0 &&
            replaced.covered == first && replaced.wrongColor == 0 &&
            CountDepthStencilMismatches(front, 0.25f) == 0;
        std::printf("  %s depth test: %d px drawn, %d kept (%d other colour) behind, %d replaced (%d other colour) in front\n",
            ok ? "ok" : "MISMATCH", first, kept.covered, kept.wrongColor, replaced.covered, replaced.wrongColor);
        CHECK(ok);
    }

    // The quads are counter-clockwise on screen, so they rely on render.cpp's
    // culling state: back faces culled, CCW front faces (DrawCelBorder sets
    // it for its draw on GL 4.1). With either one flipped every outline is
    // culled; with culling off each pixel is still drawn once.
    {
        const std::vector<CelPoint> points = SilhouetteEdge(PixelToNdc({ 30.0f, 60.0f }), PixelToNdc({ 130.0f, 60.0f }), { 0.0f, 0.5f, 0.0f });
        const glm::vec4 halfRgba(1.0f, 0.5f, 0.25f, 0.5f);
        SetCelBorderLineWidth(program, widthLocation, viewportLocation, 8.0f, kOutlineTargetWidth, kOutlineTargetHeight);
        const int drawn = CountCovered(DrawOutline(program, target, points, identity, celRgba, 2).color);
        const int clockwise = CountCovered(DrawOutline(program, target, points, identity, celRgba, 2, true,
            OutlineCull{ true, GL_BACK, GL_CW }).color);
        const int frontCulled = CountCovered(DrawOutline(program, target, points, identity, celRgba, 2, true,
            OutlineCull{ true, GL_FRONT, GL_CCW }).color);
        // Half alpha over the cleared target: a pixel drawn twice would come out at 0.75.
        const OutlineImage unculled = DrawOutline(program, target, points, identity, halfRgba, 2, true,
            OutlineCull{ false, GL_BACK, GL_CCW });
        const OutlineMeasure once = MeasureOutline(unculled.color, glm::vec4(0.5f, 0.25f, 0.125f, 0.5f),
            { 30.0f, 60.0f }, { 130.0f, 60.0f });
        const bool ok = drawn > 0 && clockwise == 0 && frontCulled == 0 && once.covered == drawn && once.wrongColor == 0;
        std::printf("  %s cull state: %d px drawn, %d with CW front faces, %d with front faces culled, "
            "%d (%d not drawn exactly once) with culling off\n",
            ok ? "ok" : "MISMATCH", drawn, clockwise, frontCulled, once.covered, once.wrongColor);
        CHECK(ok);
    }

    // render.cpp's outline setup raises no error on this context.
    while (glGetError() != GL_NO_ERROR) {}
    glLineWidth(1.0f);
    SetCelBorderLineWidth(program, widthLocation, viewportLocation, 4.125f * 1.5f,
        kOutlineTargetWidth, kOutlineTargetHeight);
    GLenum setupError = glGetError();
    std::printf("  %s outline setup: GL error 0x%04x\n", setupError == GL_NO_ERROR ? "ok" : "MISMATCH", setupError);
    CHECK(setupError == GL_NO_ERROR);
    GLfloat width = 0.0f;
    GLfloat viewport[2] = {};
    glGetUniformfv(program, widthLocation, &width);
    glGetUniformfv(program, viewportLocation, viewport);
    CHECK(width == 4.125f * 1.5f);
    CHECK(viewport[0] == static_cast<float>(kOutlineTargetWidth));
    CHECK(viewport[1] == static_cast<float>(kOutlineTargetHeight));
    // A zero-sized viewport is clamped to 1 px instead of dividing by zero.
    SetCelBorderLineWidth(program, widthLocation, viewportLocation, 4.125f, 0, -3);
    glGetUniformfv(program, viewportLocation, viewport);
    CHECK(viewport[0] == 1.0f && viewport[1] == 1.0f);

    // Why: the wide-line call the game made before this stage existed is an
    // error here, and GL_LINES stay 1 px wide.
    glLineWidth(4.125f);
    const GLenum lineWidthError = glGetError();
    std::printf("  info: glLineWidth(4.125) -> GL error 0x%04x (GL_INVALID_VALUE is 0x0501)\n", lineWidthError);
    CHECK(lineWidthError == GL_INVALID_VALUE);
    {
        std::string controlError;
        const GLuint lines = BuildProgram("celborder.vert", "celborder.frag", controlError);
        CHECK(lines != 0);
        if (lines != 0)
        {
            const std::vector<CelPoint> points = SilhouetteEdge(PixelToNdc({ 30.0f, 60.5f }), PixelToNdc({ 130.0f, 60.5f }), { 0.0f, 0.5f, 0.0f });
            const OutlineImage image = DrawOutline(lines, target, points, identity, celRgba, 2);
            const OutlineMeasure m = MeasureOutline(image.color, celRgba, { 30.0f, 60.5f }, { 130.0f, 60.5f });
            std::printf("  info: vert+frag GL_LINES after glLineWidth(4.125): thickness %.2f px\n", m.thickness[1]);
            CHECK(m.thickness[1] <= 1.5f);
            glDeleteProgram(lines);
        }
    }
    glLineWidth(1.0f);
    while (glGetError() != GL_NO_ERROR) {}

    DeleteOutlineTarget(target);
    glDeleteProgram(program);
    const GLenum glError = glGetError();
    if (glError != GL_NO_ERROR)
        std::fprintf(stderr, "celborder outline width raised GL error 0x%04x\n", glError);
    CHECK(glError == GL_NO_ERROR);
}

// GL reuses buffer names, so the sidecar must forget a deleted buffer and
// reattach a respecified one.
void TestBufferTextureSidecar()
{
    const std::vector<float> small = { 1.0f, 2.0f };
    const std::vector<float> large = { 1.0f, 2.0f, 3.0f, 4.0f, 5.0f };
    const GLuint buffer = CreateBufferWithTexture(small, kPoseWeightTextureFormat);
    const GLuint texture = GetBufferTexture(buffer);
    CHECK(texture != 0);

    // Respecify the store, as UploadGlobsetPoseWeights does when the count changes.
    glBindBuffer(GL_TEXTURE_BUFFER, buffer);
    glBufferData(GL_TEXTURE_BUFFER, static_cast<GLsizeiptr>(large.size() * sizeof(float)), large.data(), GL_DYNAMIC_DRAW);
    glBindBuffer(GL_TEXTURE_BUFFER, 0);
    // GL_TEXTURE_BUFFER_SIZE needs GL 4.3; TestCelBorderEdges reads a
    // respecified buffer through the shader instead.
    CHECK(CreateBufferTexture(buffer, kPoseWeightTextureFormat) == texture);
    CHECK(BufferTextures().size() == 1);

    DeleteBufferWithTexture(buffer);
    CHECK(GetBufferTexture(buffer) == 0);
    CHECK(BufferTextures().empty());
    CHECK(glIsTexture(texture) == GL_FALSE);
    CHECK(glGetError() == GL_NO_ERROR);
}

// ---- blip instance ranges ---------------------------------------------------

// Mirrors render.cpp's BLIPGPU: four vec4s, one per instance attribute
// (locations 1-4), which gl.cpp sets up with a 64-byte stride.
struct alignas(16) BlipInstance
{
    glm::vec4 posCreated;
    glm::vec4 velocity;
    glm::vec4 color;
    glm::vec4 roll;
};
static_assert(sizeof(BlipInstance) == 64, "gl.cpp sets the blip VAO up with a 64-byte stride");

// Mirrors blip.vert's BlipGroup block.
struct BlipGroupBlock
{
    glm::mat4 model;
    glm::vec4 params;
    glm::vec4 clqScale;
    glm::vec4 clqAlpha;
};

struct BlipVertex
{
    glm::vec4 position;
    glm::vec4 color;
};

void CaptureBlipVertex(GLuint program)
{
    const char* const varyings[] = { "gl_Position", "fragColor" };
    glTransformFeedbackVaryings(program, 2, varyings, GL_INTERLEAVED_ATTRIBS);
}

float EvaluateBlipClq(const glm::vec4& clq, float t)
{
    return clq.x + t * (clq.y + t * clq.z);
}

// Mirrors blip.vert for one corner of one instance.
BlipVertex ReferenceBlip(const BlipInstance& instance, const glm::vec2& corner, const glm::mat4& worldToClip,
    const glm::mat3& cameraMat, float currentTime, float dtFrame, const BlipGroupBlock& group)
{
    const auto safeW = [](float w) { return std::fabs(w) > 0.0001f ? w : 0.0001f; };
    const glm::vec3 cameraRight = glm::normalize(cameraMat[1]);
    const glm::vec3 cameraUp = glm::normalize(cameraMat[2]);
    const float age = currentTime - instance.posCreated.w;
    const float scale = EvaluateBlipClq(group.clqScale, age);
    const float alpha = glm::clamp(EvaluateBlipClq(group.clqAlpha, age), 0.0f, 1.0f) * group.params.x;
    const int blipMode = static_cast<int>(group.params.z);
    const glm::vec3 center = glm::vec3(group.model * glm::vec4(glm::vec3(instance.posCreated), 1.0f));
    const glm::vec3 velocity = glm::mat3(group.model) * glm::vec3(instance.velocity);

    BlipVertex result;
    result.color = glm::vec4(glm::vec3(instance.color), instance.color.a * alpha);
    if (blipMode == 1)
    {
        const glm::vec3 lineWorld = velocity * (group.params.y * dtFrame);
        const glm::vec4 centerClip = worldToClip * glm::vec4(center, 1.0f);
        const glm::vec4 headClip = worldToClip * glm::vec4(center + lineWorld, 1.0f);
        const glm::vec2 centerNdc = glm::vec2(centerClip) / safeW(centerClip.w);
        const glm::vec2 headNdc = glm::vec2(headClip) / safeW(headClip.w);
        glm::vec2 lineNdc = headNdc - centerNdc;

        const glm::vec4 rightClip = worldToClip * glm::vec4(center + cameraRight * scale, 1.0f);
        const glm::vec4 upClip = worldToClip * glm::vec4(center + cameraUp * scale, 1.0f);
        const float halfWidth = std::fmax(glm::length(glm::vec2(rightClip) / safeW(rightClip.w) - centerNdc),
            glm::length(glm::vec2(upClip) / safeW(upClip.w) - centerNdc));
        const float lineLength = glm::length(lineNdc);
        const float diameter = halfWidth * 2.0f;
        if (lineLength < diameter)
        {
            const glm::vec2 fallback = lineLength > 0.0001f ? lineNdc / lineLength : glm::vec2(0.0f, 1.0f);
            lineNdc = fallback * diameter;
        }

        glm::vec2 across(-lineNdc.y, lineNdc.x);
        const float acrossLength = glm::length(across);
        across = acrossLength > 0.0001f ? across * (halfWidth / acrossLength) : glm::vec2(halfWidth, 0.0f);
        const float along = corner.y * 0.5f + 0.5f;
        const glm::vec4 depthClip = glm::mix(centerClip, headClip, along);
        const glm::vec2 vertexNdc = centerNdc + lineNdc * along + across * corner.x;
        result.position = glm::vec4(vertexNdc * depthClip.w, depthClip.z, depthClip.w);
        return result;
    }

    glm::vec2 billboardCorner = corner;
    if (blipMode == 2)
    {
        const float s = std::sin(instance.roll.x);
        const float c = std::cos(instance.roll.x);
        billboardCorner = glm::mat2(c, s, -s, c) * billboardCorner;
    }
    const glm::vec3 worldPos = center + (cameraRight * billboardCorner.x + cameraUp * billboardCorner.y) * scale;
    result.position = worldToClip * glm::vec4(worldPos, 1.0f);
    return result;
}

struct BlipRange
{
    GLuint baseInstance;
    GLsizei instanceCount;
};


// Draws two blip instance ranges with DrawBlipInstances, as render.cpp's
// DrawStagedBlips does, through a VAO set up like gl.cpp's, and checks each
// range's vertices against blip.vert's reference for that range's own
// instances. glDrawElementsInstancedBaseInstance, which the 4.3 path uses, is
// GL 4.2 and null here.
void TestBlipInstanceRanges()
{
    std::printf("info: glDrawElementsInstancedBaseInstance %s\n",
        glad_glDrawElementsInstancedBaseInstance != nullptr ? "loaded" : "null");
    std::string error;
    const GLuint program = BuildProgram("blip.vert", "blip.frag", error, CaptureBlipVertex);
    if (program == 0)
    {
        std::fprintf(stderr, "FAIL: blip did not build with transform feedback: %s\n", error.c_str());
        CHECK(program != 0);
        return;
    }
    glUseProgram(program);
    while (glGetError() != GL_NO_ERROR) {}

    // gl.cpp's instanced blip quad.
    const float corners[] = { -1.0f, -1.0f, 1.0f, -1.0f, 1.0f, 1.0f, -1.0f, 1.0f };
    const uint16_t indices[] = { 0, 1, 2, 0, 2, 3 };

    std::vector<BlipInstance> instances(5);
    for (size_t i = 0; i < instances.size(); ++i)
    {
        const float f = static_cast<float>(i);
        instances[i].posCreated = glm::vec4(10.0f * f + 1.0f, 2.0f * f, -f, 0.25f * f);
        instances[i].velocity = glm::vec4(3.0f + f, 0.5f * f, 0.0f, 0.0f);
        instances[i].color = glm::vec4(0.1f * f, 0.2f, 0.3f, 0.5f + 0.1f * f);
        instances[i].roll = glm::vec4(0.3f * f + 0.1f, 0.0f, 0.0f, 0.0f);
    }

    const GLsizei stride = static_cast<GLsizei>(sizeof(BlipInstance));
    GLuint vao = 0;
    GLuint cornerVbo = 0;
    GLuint ebo = 0;
    GLuint instanceVbo = 0;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &cornerVbo);
    glGenBuffers(1, &ebo);
    glGenBuffers(1, &instanceVbo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, cornerVbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(corners), corners, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), nullptr);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, instanceVbo);
    glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(instances.size() * sizeof(BlipInstance)),
        instances.data(), GL_STREAM_DRAW);
    for (GLuint attribute = 1; attribute <= 4; ++attribute)
    {
        glEnableVertexAttribArray(attribute);
        glVertexAttribPointer(attribute, 4, GL_FLOAT, GL_FALSE, 64,
            reinterpret_cast<void*>(static_cast<uintptr_t>((attribute - 1) * 16)));
        glVertexAttribDivisor(attribute, 1);
    }

    const glm::mat4 worldToClip = glm::translate(glm::scale(glm::mat4(1.0f), glm::vec3(0.05f)),
        glm::vec3(-2.0f, 1.0f, 0.5f));
    const glm::mat3 cameraMat(1.0f);
    const float currentTime = 2.0f;
    const float dtFrame = 0.5f;
    struct CmglBlock
    {
        glm::mat4 matWorldToClip;
        glm::vec4 cameraPos;
    } cmgl = { worldToClip, glm::vec4(0.0f) };
    const GLuint cmglBuffer = CreateUniformBuffer(program, "CMGL", FindBlockBinding("CMGL"), &cmgl, sizeof(cmgl));
    glUniformMatrix3fv(glGetUniformLocation(program, "cameraMat"), 1, GL_FALSE, &cameraMat[0][0]);
    glUniform1f(glGetUniformLocation(program, "currentTime"), currentTime);
    glUniform1f(glGetUniformLocation(program, "dtFrame"), dtFrame);

    // Second range first, so a draw that ignores baseInstance reads
    // instances 0-2 and mismatches.
    const BlipRange ranges[] = { { 2, 3 }, { 0, 2 } };
    GLsizei totalInstances = 0;
    for (const BlipRange& range : ranges)
        totalInstances += range.instanceCount;
    const size_t vertexCount = static_cast<size_t>(totalInstances) * 6;

    // Billboard, flying (reads velocity) and rolled (reads roll) modes cover
    // all four instance attributes.
    for (int blipMode = 0; blipMode <= 2; ++blipMode)
    {
        const BlipGroupBlock group = { glm::mat4(1.0f), glm::vec4(1.0f, 1.0f, static_cast<float>(blipMode), 0.0f),
            glm::vec4(0.5f, 0.1f, 0.0f, 0.0f), glm::vec4(0.8f, 0.0f, 0.0f, 0.0f) };
        const GLuint groupBuffer = CreateUniformBuffer(program, "BlipGroup", FindBlockBinding("BlipGroup"),
            &group, sizeof(group));

        const GLsizeiptr captureSize = static_cast<GLsizeiptr>(vertexCount * sizeof(BlipVertex));
        GLuint capture = 0;
        glGenBuffers(1, &capture);
        glBindBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, capture);
        std::vector<BlipVertex> captured(vertexCount, BlipVertex{ glm::vec4(-999.0f), glm::vec4(-999.0f) });
        glBufferData(GL_TRANSFORM_FEEDBACK_BUFFER, captureSize, captured.data(), GL_STATIC_READ);
        glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, capture);

        // DrawStagedBlips runs with the blip VAO and instance buffer bound.
        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, instanceVbo);
        glEnable(GL_RASTERIZER_DISCARD);
        glBeginTransformFeedback(GL_TRIANGLES);
        for (const BlipRange& range : ranges)
            DrawBlipInstances(stride, range.baseInstance, range.instanceCount);
        glEndTransformFeedback();
        glDisable(GL_RASTERIZER_DISCARD);
        PointBlipInstanceAttributes(stride, 0);

        glBindBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, capture);
        glGetBufferSubData(GL_TRANSFORM_FEEDBACK_BUFFER, 0, captureSize, captured.data());
        glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, 0);
        glDeleteBuffers(1, &capture);
        glDeleteBuffers(1, &groupBuffer);

        int mismatches = 0;
        size_t vertex = 0;
        for (const BlipRange& range : ranges)
        {
            for (GLsizei i = 0; i < range.instanceCount; ++i)
            {
                const BlipInstance& instance = instances[range.baseInstance + static_cast<GLuint>(i)];
                for (const uint16_t index : indices)
                {
                    const glm::vec2 corner(corners[index * 2], corners[index * 2 + 1]);
                    const BlipVertex expected = ReferenceBlip(instance, corner, worldToClip, cameraMat,
                        currentTime, dtFrame, group);
                    const BlipVertex& actual = captured[vertex++];
                    if (!NearlyEqual(actual.position, expected.position) || !NearlyEqual(actual.color, expected.color))
                    {
                        ++mismatches;
                        std::fprintf(stderr, "  MISMATCH mode %d range base %u instance %u: got (%.5f, %.5f, %.5f, %.5f) "
                            "color (%.3f, %.3f, %.3f, %.3f) want (%.5f, %.5f, %.5f, %.5f) color (%.3f, %.3f, %.3f, %.3f)\n",
                            blipMode, range.baseInstance, range.baseInstance + static_cast<GLuint>(i),
                            actual.position.x, actual.position.y, actual.position.z, actual.position.w,
                            actual.color.x, actual.color.y, actual.color.z, actual.color.w,
                            expected.position.x, expected.position.y, expected.position.z, expected.position.w,
                            expected.color.x, expected.color.y, expected.color.z, expected.color.w);
                    }
                }
            }
        }
        std::printf("  %s blip mode %d: %zu vertices from ranges (base 2, 3 instances) and (base 0, 2 instances), "
            "%d mismatches\n", mismatches == 0 ? "ok" : "MISMATCH", blipMode, vertexCount, mismatches);
        CHECK(mismatches == 0);
    }

    // The ranges leave the VAO as gl.cpp set it up: instance 0 at offsets 0/16/32/48.
    glBindVertexArray(vao);
    for (GLuint attribute = 1; attribute <= 4; ++attribute)
    {
        void* pointer = nullptr;
        glGetVertexAttribPointerv(attribute, GL_VERTEX_ATTRIB_ARRAY_POINTER, &pointer);
        const uintptr_t offset = reinterpret_cast<uintptr_t>(pointer);
        std::printf("  blip attribute %u offset after the ranges: %zu (expected %u)\n",
            attribute, static_cast<size_t>(offset), (attribute - 1) * 16);
        CHECK(offset == (attribute - 1) * 16);
    }
    CHECK(glGetError() == GL_NO_ERROR);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBufferBase(GL_UNIFORM_BUFFER, FindBlockBinding("CMGL"), 0);
    glBindBufferBase(GL_UNIFORM_BUFFER, FindBlockBinding("BlipGroup"), 0);
    glDeleteBuffers(1, &cmglBuffer);
    glDeleteBuffers(1, &instanceVbo);
    glDeleteBuffers(1, &ebo);
    glDeleteBuffers(1, &cornerVbo);
    glDeleteVertexArrays(1, &vao);
    glUseProgram(0);
    glDeleteProgram(program);
}

// ---- glob shadow fetch helpers ----------------------------------------------

// Mirrors shadow.h's SHADOWBLK, SHADOWSSBOHEADER and ACTIVESHADOWS, which this
// GL-only test can't include. The asserts repeat shadow.h's, so a drift in
// either copy from the 14-texel map below fails to compile.
struct ShadowBlk
{
    glm::mat4 matWorldToUv;
    glm::mat4 matClipToUv;
    glm::vec4 rgba;
    float wMin;
    float wMax;
    float gReserved;
    float wFadeMin;
    int textureSlot;
    int clampS;
    int clampT;
    int _pad0;
    glm::vec4 posEffect;
    float sRadiusEffect;
    int   fDynamic;
    int   shdk;
    int   _pad1;
    glm::vec4 normalCast;
};
static_assert(sizeof(ShadowBlk) == 224, "SHADOWBLK is 14 RGBA32UI texels");
static_assert(offsetof(ShadowBlk, matWorldToUv) == 0, "texels 0-3");
static_assert(offsetof(ShadowBlk, matClipToUv) == 64, "texels 4-7");
static_assert(offsetof(ShadowBlk, rgba) == 128, "texel 8");
static_assert(offsetof(ShadowBlk, wMin) == 144, "texel 9.x");
static_assert(offsetof(ShadowBlk, wMax) == 148, "texel 9.y");
static_assert(offsetof(ShadowBlk, gReserved) == 152, "texel 9.z");
static_assert(offsetof(ShadowBlk, wFadeMin) == 156, "texel 9.w");
static_assert(offsetof(ShadowBlk, textureSlot) == 160, "texel 10.x");
static_assert(offsetof(ShadowBlk, clampS) == 164, "texel 10.y");
static_assert(offsetof(ShadowBlk, clampT) == 168, "texel 10.z");
static_assert(offsetof(ShadowBlk, _pad0) == 172, "texel 10.w");
static_assert(offsetof(ShadowBlk, posEffect) == 176, "texel 11");
static_assert(offsetof(ShadowBlk, sRadiusEffect) == 192, "texel 12.x");
static_assert(offsetof(ShadowBlk, fDynamic) == 196, "texel 12.y");
static_assert(offsetof(ShadowBlk, shdk) == 200, "texel 12.z");
static_assert(offsetof(ShadowBlk, _pad1) == 204, "texel 12.w");
static_assert(offsetof(ShadowBlk, normalCast) == 208, "texel 13");

struct alignas(16) ShadowSsboHeader
{
    int numShadows;
    int pad[3];
};
static_assert(sizeof(ShadowSsboHeader) == 16, "the header is texel 0");

constexpr int kMaxShadows = 255; // shadow.h's MAX_SHADOWS
struct ActiveShadows
{
    int numShadows;
    int shadowsIndices[kMaxShadows];
};
static_assert(sizeof(ActiveShadows) == 1024, "ACTIVESHADOWS is 256 R32I texels");

constexpr int kShadowTexels = 14;
constexpr int kShadowWords = kShadowTexels * 4;

// The 56 words FetchShadow must return, field by field in declaration order.
std::array<uint32_t, kShadowWords> ShadowWords(const ShadowBlk& shadow)
{
    std::array<uint32_t, kShadowWords> words{};
    int next = 0;
    const auto add = [&](auto value)
    {
        static_assert(sizeof(value) == 4, "one word per field");
        std::memcpy(&words[next++], &value, 4);
    };
    for (const glm::mat4* matrix : { &shadow.matWorldToUv, &shadow.matClipToUv })
        for (int column = 0; column < 4; ++column)
            for (int row = 0; row < 4; ++row)
                add((*matrix)[column][row]);
    for (int i = 0; i < 4; ++i) add(shadow.rgba[i]);
    add(shadow.wMin); add(shadow.wMax); add(shadow.gReserved); add(shadow.wFadeMin);
    add(shadow.textureSlot); add(shadow.clampS); add(shadow.clampT); add(shadow._pad0);
    for (int i = 0; i < 4; ++i) add(shadow.posEffect[i]);
    add(shadow.sRadiusEffect); add(shadow.fDynamic); add(shadow.shdk); add(shadow._pad1);
    for (int i = 0; i < 4; ++i) add(shadow.normalCast[i]);
    return words;
}

const char* const kShadowTexelNames[kShadowTexels] =
{
    "matWorldToUv[0]", "matWorldToUv[1]", "matWorldToUv[2]", "matWorldToUv[3]",
    "matClipToUv[0]", "matClipToUv[1]", "matClipToUv[2]", "matClipToUv[3]",
    "rgba", "wMin..wFadeMin", "textureSlot..padTexture2", "posEffect",
    "sRadiusEffect..._pad1", "normalCast",
};

// Returns the text between "// probe:begin <name>" and "// probe:end <name>".
std::string ExtractProbeRegion(const std::string& source, const std::string& name)
{
    const std::string begin = "// probe:begin " + name + "\n";
    const std::string end = "// probe:end " + name + "\n";
    const size_t start = source.find(begin);
    if (start == std::string::npos)
        return {};
    const size_t stop = source.find(end, start);
    if (stop == std::string::npos)
        return {};
    return source.substr(start + begin.size(), stop - start - begin.size());
}

// Returns the "uniform <type> <name>;" line, or an empty string.
std::string ExtractUniformDeclaration(const std::string& source, const std::string& name)
{
    std::istringstream lines(source);
    const std::regex declaration("^\\s*uniform\\s+\\w+\\s+" + name + "\\s*;");
    for (std::string line; std::getline(lines, line);)
    {
        if (std::regex_search(line, declaration))
            return line + "\n";
    }
    return {};
}

const char* const kShadowProbeVaryings[] =
{
    "probeTexel0", "probeTexel1", "probeTexel2", "probeTexel3", "probeTexel4", "probeTexel5", "probeTexel6",
    "probeTexel7", "probeTexel8", "probeTexel9", "probeTexel10", "probeTexel11", "probeTexel12", "probeTexel13",
    "probeCounts",
};
constexpr int kShadowProbeVaryingCount = static_cast<int>(std::size(kShadowProbeVaryings));

void CaptureShadowProbe(GLuint program)
{
    glTransformFeedbackVaryings(program, kShadowProbeVaryingCount, kShadowProbeVaryings, GL_INTERLEAVED_ATTRIBS);
}

// Builds a vertex shader from the helpers `file` declares. Vertex i fetches
// shadow i and writes every field, ints through intBitsToFloat so their bits
// survive; probeCounts holds ShadowCount() and, when the file has them,
// ActiveShadowCount() and the active indices i and MAX_SHADOWS - 1 - i.
std::string BuildShadowProbeSource(const char* file, bool withActive, std::string& error)
{
    const std::string source = CaneEmbeddedShader(file);
    std::string probe = std::string("#version ") + kShaderVersionGL41 + "\n";

    const auto addRegion = [&](const char* name)
    {
        const std::string region = ExtractProbeRegion(source, name);
        if (region.empty())
            error += std::string(file) + " has no probe region " + name + "; ";
        probe += region;
    };
    const auto addSampler = [&](const char* name)
    {
        const std::string declaration = ExtractUniformDeclaration(source, name);
        if (declaration.empty())
            error += std::string(file) + " declares no sampler " + name + "; ";
        probe += declaration;
    };

    addRegion("SHADOW");
    addSampler("shadowTex");
    if (withActive)
        addSampler("activeShadowTex");
    addRegion("ShadowCount");
    addRegion("FetchShadow");
    if (withActive)
    {
        addRegion("ActiveShadowCount");
        addRegion("ActiveShadowIndex");
    }

    for (int i = 0; i < kShadowTexels; ++i)
        probe += "out vec4 probeTexel" + std::to_string(i) + ";\n";
    probe += "out vec4 probeCounts;\n";
    probe += R"(
void main()
{
    SHADOW shadow = FetchShadow(gl_VertexID);
    probeTexel0 = shadow.matWorldToUv[0];
    probeTexel1 = shadow.matWorldToUv[1];
    probeTexel2 = shadow.matWorldToUv[2];
    probeTexel3 = shadow.matWorldToUv[3];
    probeTexel4 = shadow.matClipToUv[0];
    probeTexel5 = shadow.matClipToUv[1];
    probeTexel6 = shadow.matClipToUv[2];
    probeTexel7 = shadow.matClipToUv[3];
    probeTexel8 = shadow.rgba;
    probeTexel9 = vec4(shadow.wMin, shadow.wMax, shadow.gReserved, shadow.wFadeMin);
    probeTexel10 = intBitsToFloat(ivec4(shadow.textureSlot, shadow.clampS, shadow.clampT, shadow.padTexture2));
    probeTexel11 = shadow.posEffect;
    probeTexel12 = vec4(shadow.sRadiusEffect, intBitsToFloat(ivec3(shadow.fDynamic, shadow.shdk, shadow._pad1)));
    probeTexel13 = shadow.normalCast;
)";
    probe += withActive
        ? "    probeCounts = intBitsToFloat(ivec4(ShadowCount(), ActiveShadowCount(),\n"
          "        ActiveShadowIndex(gl_VertexID), ActiveShadowIndex(" + std::to_string(kMaxShadows - 1) + " - gl_VertexID)));\n"
        : "    probeCounts = intBitsToFloat(ivec4(ShadowCount(), 0, 0, 0));\n";
    probe += "}\n";
    return probe;
}

// Uploads the records the way shadow.cpp's AllocateShadows does: the store,
// its texture, then the header and the SHADOWBLK array, via GL_TEXTURE_BUFFER. `extraTexels` appends
// a partial record the count must ignore.
GLuint CreateShadowBuffer(int headerCount, const std::vector<ShadowBlk>& shadows, int extraTexels)
{
    const GLsizeiptr headerSize = sizeof(ShadowSsboHeader);
    const GLsizeiptr shadowSize = static_cast<GLsizeiptr>(sizeof(ShadowBlk) * shadows.size());
    const GLsizeiptr extraSize = static_cast<GLsizeiptr>(extraTexels) * 16;
    std::vector<unsigned char> extra(static_cast<size_t>(extraSize), 0x5A);

    GLuint buffer = 0;
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_TEXTURE_BUFFER, buffer);
    glBufferData(GL_TEXTURE_BUFFER, headerSize + shadowSize + extraSize, nullptr, GL_DYNAMIC_DRAW);
    CHECK(CreateBufferTexture(buffer, kShadowTextureFormat) != 0);
    ShadowSsboHeader header = {};
    header.numShadows = headerCount;
    glBufferSubData(GL_TEXTURE_BUFFER, 0, headerSize, &header);
    glBufferSubData(GL_TEXTURE_BUFFER, headerSize, shadowSize, shadows.data());
    if (extraSize > 0)
        glBufferSubData(GL_TEXTURE_BUFFER, headerSize + shadowSize, extraSize, extra.data());
    glBindBuffer(GL_TEXTURE_BUFFER, 0);
    return buffer;
}

GLenum UniformType(GLuint program, const char* wanted)
{
    GLint uniformCount = 0;
    glGetProgramiv(program, GL_ACTIVE_UNIFORMS, &uniformCount);
    for (GLint i = 0; i < uniformCount; ++i)
    {
        char name[64] = {};
        GLint size = 0;
        GLenum type = GL_NONE;
        glGetActiveUniform(program, static_cast<GLuint>(i), sizeof(name), nullptr, &size, &type, name);
        if (std::string(name) == wanted)
            return type;
    }
    return GL_NONE;
}

// Draws one point per record with the textures bound the way shadow.cpp's
// BindSwShadowTextures binds them and returns the captured words.
std::vector<std::array<uint32_t, kShadowProbeVaryingCount * 4>> RunShadowProbe(GLuint program, int vertexCount,
    GLuint shadowBuffer, GLuint activeBuffer)
{
    glActiveTexture(GL_TEXTURE0 + kShadowTextureUnit);
    glBindTexture(GL_TEXTURE_BUFFER, GetBufferTexture(shadowBuffer));
    glActiveTexture(GL_TEXTURE0 + kActiveShadowTextureUnit);
    glBindTexture(GL_TEXTURE_BUFFER, GetBufferTexture(activeBuffer));
    glActiveTexture(GL_TEXTURE0);

    using Capture = std::array<uint32_t, kShadowProbeVaryingCount * 4>;
    GLuint vao = 0;
    GLuint capture = 0;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    const GLsizeiptr captureSize = static_cast<GLsizeiptr>(vertexCount * sizeof(Capture));
    glGenBuffers(1, &capture);
    glBindBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, capture);
    glBufferData(GL_TRANSFORM_FEEDBACK_BUFFER, captureSize, nullptr, GL_STATIC_READ);
    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, capture);

    glEnable(GL_RASTERIZER_DISCARD);
    glBeginTransformFeedback(GL_POINTS);
    glDrawArrays(GL_POINTS, 0, vertexCount);
    glEndTransformFeedback();
    glDisable(GL_RASTERIZER_DISCARD);

    Capture filler;
    filler.fill(0xDEADBEEFu);
    std::vector<Capture> captured(static_cast<size_t>(vertexCount), filler);
    glBindBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, capture);
    glGetBufferSubData(GL_TRANSFORM_FEEDBACK_BUFFER, 0, captureSize, captured.data());

    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, 0);
    glBindVertexArray(0);
    glDeleteBuffers(1, &capture);
    glDeleteVertexArrays(1, &vao);
    for (GLint unit : { kShadowTextureUnit, kActiveShadowTextureUnit })
    {
        glActiveTexture(GL_TEXTURE0 + unit);
        glBindTexture(GL_TEXTURE_BUFFER, 0);
    }
    glActiveTexture(GL_TEXTURE0);
    return captured;
}

std::vector<ShadowBlk> MakeProbeShadows()
{
    std::vector<ShadowBlk> shadows(3);
    // Non-symmetric matrices so a transposed or column-swapped read shows;
    // -0.0f and negative entries so a sign or bit loss shows.
    shadows[0].matWorldToUv = glm::translate(glm::mat4(1.0f), glm::vec3(0.5f, -2.25f, 7.0f)) *
        glm::rotate(glm::mat4(1.0f), 0.7f, glm::normalize(glm::vec3(1.0f, -2.0f, 0.5f))) *
        glm::scale(glm::mat4(1.0f), glm::vec3(1.5f, -0.75f, 2.0f));
    shadows[0].matClipToUv = glm::mat4(
        glm::vec4(0.5f, 0.0f, -0.0f, 1.25f), glm::vec4(-3.0f, 0.5f, 0.125f, 0.0f),
        glm::vec4(0.0f, 9.5f, -0.5f, 2.0f), glm::vec4(0.5f, -0.5f, 1.0f, -7.75f));
    shadows[1].matWorldToUv = glm::perspective(glm::radians(50.0f), 1.3f, 0.25f, 300.0f) *
        glm::lookAt(glm::vec3(-4.0f, 12.0f, 3.0f), glm::vec3(1.0f, 0.0f, -2.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    shadows[1].matClipToUv = glm::transpose(shadows[0].matWorldToUv) * -2.0f;
    shadows[2].matWorldToUv = glm::rotate(glm::mat4(1.0f), -2.1f, glm::normalize(glm::vec3(0.3f, 0.4f, -1.0f))) +
        glm::mat4(glm::vec4(1.0f, 2.0f, 3.0f, 4.0f), glm::vec4(5.0f, 6.0f, 7.0f, 8.0f),
                  glm::vec4(9.0f, 10.0f, 11.0f, 12.0f), glm::vec4(13.0f, 14.0f, 15.0f, 16.0f));
    shadows[2].matClipToUv = glm::scale(glm::mat4(1.0f), glm::vec3(-1e6f, 3e-5f, 0.333f));

    struct Ints { int textureSlot, clampS, clampT, pad0, fDynamic, shdk, pad1; };
    const Ints ints[3] =
    {
        { 0, 1, 0, 0x12345678, 0, 1, 0 },
        { 1, 0, 1, -2, 1, 2, -1 },
        { -1, 1000000000, -1000000000, 2147483647, -5, 0, 7 },
    };
    for (int i = 0; i < 3; ++i)
    {
        ShadowBlk& shadow = shadows[i];
        const float f = static_cast<float>(i + 1);
        shadow.rgba = glm::vec4(0.25f * f, -0.5f * f, 1.75f, 0.125f / f);
        shadow.wMin = -3.5f * f;
        shadow.wMax = 1024.25f + f;
        shadow.gReserved = -0.0625f * f;
        shadow.wFadeMin = 0.001f * f;
        shadow.textureSlot = ints[i].textureSlot;
        shadow.clampS = ints[i].clampS;
        shadow.clampT = ints[i].clampT;
        shadow._pad0 = ints[i].pad0;
        shadow.posEffect = glm::vec4(-12.5f * f, 3.0f, 0.75f * f, 1.0f);
        shadow.sRadiusEffect = 40.5f / f;
        shadow.fDynamic = ints[i].fDynamic;
        shadow.shdk = ints[i].shdk;
        shadow._pad1 = ints[i].pad1;
        shadow.normalCast = glm::vec4(0.0f, -0.6f * f, 0.8f, -0.0f);
    }
    return shadows;
}

int AsInt(uint32_t word)
{
    int value = 0;
    std::memcpy(&value, &word, 4);
    return value;
}

void TestGlobShadowFetch(const char* file, bool withActive)
{
    std::printf("glob shadow fetch helpers from %s (transform feedback vs C++ layout):\n", file);
    std::string error;
    const std::string source = BuildShadowProbeSource(file, withActive, error);
    if (!error.empty())
    {
        std::fprintf(stderr, "FAIL: %s\n", error.c_str());
        CHECK(error.empty());
        return;
    }
    const GLuint program = LinkStages(CompileSource(GL_VERTEX_SHADER, file, source),
        CompileSource(GL_FRAGMENT_SHADER, "probe.frag", std::string("#version ") + kShaderVersionGL41 + "\nvoid main() {}\n"),
        error, CaptureShadowProbe);
    if (program == 0)
    {
        std::fprintf(stderr, "FAIL: %s shadow probe did not build: %s\n", file, error.c_str());
        CHECK(program != 0);
        return;
    }
    CHECK(CheckSamplerUnits(file, program) == (withActive ? 2 : 1));
    // Records mix ints and floats, so they must be read as integers; see kShadowTextureFormat.
    const GLenum shadowTexType = UniformType(program, "shadowTex");
    std::printf("  shadowTex type 0x%04x (want GL_UNSIGNED_INT_SAMPLER_BUFFER 0x%04x)\n",
        shadowTexType, GL_UNSIGNED_INT_SAMPLER_BUFFER);
    CHECK(shadowTexType == GL_UNSIGNED_INT_SAMPLER_BUFFER);
    if (withActive)
    {
        const GLenum activeTexType = UniformType(program, "activeShadowTex");
        std::printf("  activeShadowTex type 0x%04x (want GL_INT_SAMPLER_BUFFER 0x%04x)\n",
            activeTexType, GL_INT_SAMPLER_BUFFER);
        CHECK(activeTexType == GL_INT_SAMPLER_BUFFER);
    }

    const std::vector<ShadowBlk> shadows = MakeProbeShadows();
    const int recordCount = static_cast<int>(shadows.size());

    // Built the way AllocateShadows builds it: glBufferData, then the texture.
    ActiveShadows active = {};
    active.numShadows = 4;
    active.shadowsIndices[0] = 2;
    active.shadowsIndices[1] = 0;
    active.shadowsIndices[2] = -1;
    active.shadowsIndices[kMaxShadows - 3] = -7;
    active.shadowsIndices[kMaxShadows - 2] = 1000000;
    active.shadowsIndices[kMaxShadows - 1] = 77;
    GLuint activeBuffer = 0;
    glGenBuffers(1, &activeBuffer);
    glBindBuffer(GL_TEXTURE_BUFFER, activeBuffer);
    glBufferData(GL_TEXTURE_BUFFER, sizeof(ActiveShadows), &active, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_TEXTURE_BUFFER, 0);
    CHECK(CreateBufferTexture(activeBuffer, kActiveShadowTextureFormat) != 0);

    struct CountCase
    {
        const char* name;
        int header;
        int extraTexels;
        int expected;
    };
    const CountCase kCountCases[] =
    {
        { "header == records", recordCount, 0, recordCount },
        { "header above records", recordCount + 4, 0, recordCount },
        { "header below records", recordCount - 1, 0, recordCount - 1 },
        { "header zero", 0, 0, 0 },
        { "header negative", -4, 0, 0 },
        { "partial trailing record", recordCount + 6, kShadowTexels - 1, recordCount },
    };

    glUseProgram(program);
    for (const CountCase& countCase : kCountCases)
    {
        const GLuint shadowBuffer = CreateShadowBuffer(countCase.header, shadows, countCase.extraTexels);
        const auto captured = RunShadowProbe(program, recordCount, shadowBuffer, activeBuffer);
        for (int record = 0; record < recordCount; ++record)
        {
            const std::array<uint32_t, kShadowWords> expected = ShadowWords(shadows[record]);
            int mismatches = 0;
            for (int texel = 0; texel < kShadowTexels; ++texel)
            {
                const uint32_t* got = &captured[record][texel * 4];
                const uint32_t* want = &expected[texel * 4];
                if (std::memcmp(got, want, 16) == 0)
                    continue;
                ++mismatches;
                std::printf("  MISMATCH %s record %d texel %d %s: got %08x %08x %08x %08x want %08x %08x %08x %08x\n",
                    countCase.name, record, texel, kShadowTexelNames[texel],
                    got[0], got[1], got[2], got[3], want[0], want[1], want[2], want[3]);
            }
            CHECK(mismatches == 0);

            const uint32_t* counts = &captured[record][kShadowTexels * 4];
            const int shadowCount = AsInt(counts[0]);
            if (record == 0)
                std::printf("  %s %s: ShadowCount() %d (expected %d)\n",
                    shadowCount == countCase.expected ? "ok" : "MISMATCH", countCase.name, shadowCount, countCase.expected);
            CHECK(shadowCount == countCase.expected);
            if (!withActive)
                continue;
            const int activeCount = AsInt(counts[1]);
            const int first = AsInt(counts[2]);
            const int last = AsInt(counts[3]);
            const int wantFirst = active.shadowsIndices[record];
            const int wantLast = active.shadowsIndices[kMaxShadows - 1 - record];
            if (countCase.header == recordCount)
                std::printf("  %s ActiveShadowCount() %d, ActiveShadowIndex(%d) %d, ActiveShadowIndex(%d) %d"
                    " (expected %d, %d, %d)\n",
                    activeCount == active.numShadows && first == wantFirst && last == wantLast ? "ok" : "MISMATCH",
                    activeCount, record, first, kMaxShadows - 1 - record, last, active.numShadows, wantFirst, wantLast);
            CHECK(activeCount == active.numShadows);
            CHECK(first == wantFirst);
            CHECK(last == wantLast);
        }
        if (countCase.header == recordCount)
            std::printf("  records compared bit-exactly: %d x %d texels\n", recordCount, kShadowTexels);
        DeleteBufferWithTexture(shadowBuffer);
    }

    const GLenum glError = glGetError();
    if (glError != GL_NO_ERROR)
        std::fprintf(stderr, "%s shadow probe raised GL error 0x%04x\n", file, glError);
    CHECK(glError == GL_NO_ERROR);

    glUseProgram(0);
    DeleteBufferWithTexture(activeBuffer);
    CHECK(BufferTextures().empty());
    glDeleteProgram(program);
}

// ---- glob light fetch helpers -----------------------------------------------

// Mirrors light.h's LIGHTBLK, LightSSBOHeader and ACTIVELIGHTS, which this
// GL-only test can't include. The asserts repeat light.h's, so a drift in
// either copy from the 13-texel map below fails to compile.
struct LightBlk
{
    int lightk;
    int fExcludeDynamicObjects;
    int fActive;
    int fDynamic;
    glm::vec4 pos;
    glm::vec4 dir;
    glm::vec4 color;
    float constant;
    float invDst;
    float pad4;
    float dst;
    glm::vec4 ru;
    glm::vec4 du;
    glm::mat4 matFrustrum;
    glm::vec4 falloffScale;
    glm::vec4 falloffBias;
};
static_assert(sizeof(LightBlk) == 208, "LIGHTBLK is 13 RGBA32UI texels");
static_assert(offsetof(LightBlk, lightk) == 0, "texel 0.x");
static_assert(offsetof(LightBlk, fExcludeDynamicObjects) == 4, "texel 0.y");
static_assert(offsetof(LightBlk, fActive) == 8, "texel 0.z");
static_assert(offsetof(LightBlk, fDynamic) == 12, "texel 0.w");
static_assert(offsetof(LightBlk, pos) == 16, "texel 1");
static_assert(offsetof(LightBlk, dir) == 32, "texel 2");
static_assert(offsetof(LightBlk, color) == 48, "texel 3");
static_assert(offsetof(LightBlk, constant) == 64, "texel 4.x");
static_assert(offsetof(LightBlk, invDst) == 68, "texel 4.y");
static_assert(offsetof(LightBlk, pad4) == 72, "texel 4.z");
static_assert(offsetof(LightBlk, dst) == 76, "texel 4.w");
static_assert(offsetof(LightBlk, ru) == 80, "texel 5");
static_assert(offsetof(LightBlk, du) == 96, "texel 6");
static_assert(offsetof(LightBlk, matFrustrum) == 112, "texels 7-10");
static_assert(offsetof(LightBlk, falloffScale) == 176, "texel 11");
static_assert(offsetof(LightBlk, falloffBias) == 192, "texel 12");

struct alignas(16) LightSsboHeader
{
    int numLights;
    int pad[3];
};
static_assert(sizeof(LightSsboHeader) == 16, "the header is lightTex texel 0");

constexpr int kMaxLights = 256; // light.h's MAX_LIGHTS
struct ActiveLights
{
    int numStaticLights;
    int staticLightIndices[kMaxLights];
    int numDynamicLights;
    int dynamicLightIndices[kMaxLights];
};
static_assert(sizeof(ActiveLights) == 2056, "ACTIVELIGHTS is 514 R32I texels");
static_assert(offsetof(ActiveLights, staticLightIndices) == 4, "texels 1-256");
static_assert(offsetof(ActiveLights, numDynamicLights) == 1028, "texel 257");
static_assert(offsetof(ActiveLights, dynamicLightIndices) == 1032, "texels 258-513");

constexpr int kLightTexels = 13;
constexpr int kLightWords = kLightTexels * 4;

// The 52 words FetchLight must return, field by field in declaration order.
std::array<uint32_t, kLightWords> LightWords(const LightBlk& light)
{
    std::array<uint32_t, kLightWords> words{};
    int next = 0;
    const auto add = [&](auto value)
    {
        static_assert(sizeof(value) == 4, "one word per field");
        std::memcpy(&words[next++], &value, 4);
    };
    add(light.lightk); add(light.fExcludeDynamicObjects); add(light.fActive); add(light.fDynamic);
    for (const glm::vec4* vector : { &light.pos, &light.dir, &light.color })
        for (int i = 0; i < 4; ++i)
            add((*vector)[i]);
    add(light.constant); add(light.invDst); add(light.pad4); add(light.dst);
    for (const glm::vec4* vector : { &light.ru, &light.du })
        for (int i = 0; i < 4; ++i)
            add((*vector)[i]);
    for (int column = 0; column < 4; ++column)
        for (int row = 0; row < 4; ++row)
            add(light.matFrustrum[column][row]);
    for (const glm::vec4* vector : { &light.falloffScale, &light.falloffBias })
        for (int i = 0; i < 4; ++i)
            add((*vector)[i]);
    return words;
}

const char* const kLightTexelNames[kLightTexels] =
{
    "lightk..fDynamic", "pos", "dir", "color", "constant..dst", "ru", "du",
    "matFrustrum[0]", "matFrustrum[1]", "matFrustrum[2]", "matFrustrum[3]", "falloffScale", "falloffBias",
};

// Returns the "#define <name> ..." line, or an empty string.
std::string ExtractDefine(const std::string& source, const std::string& name)
{
    std::istringstream lines(source);
    const std::regex definition("^\\s*#define\\s+" + name + "\\s");
    for (std::string line; std::getline(lines, line);)
    {
        if (std::regex_search(line, definition))
            return line + "\n";
    }
    return {};
}

// Ints are captured through integer outputs so no float conversion can touch
// their bits; float fields come back as their bit patterns.
const char* const kLightProbeVaryings[] =
{
    "probeFlags", "probeTexel1", "probeTexel2", "probeTexel3", "probeTexel4", "probeTexel5", "probeTexel6",
    "probeTexel7", "probeTexel8", "probeTexel9", "probeTexel10", "probeTexel11", "probeTexel12",
    "probeCounts", "probeActive",
};
constexpr int kLightProbeVaryingCount = static_cast<int>(std::size(kLightProbeVaryings));

void CaptureLightProbe(GLuint program)
{
    glTransformFeedbackVaryings(program, kLightProbeVaryingCount, kLightProbeVaryings, GL_INTERLEAVED_ATTRIBS);
}

// Builds a vertex shader from glob.vert's light helpers. Vertex v fetches
// light v % probeRecords (a TV light when probeTv is set) and writes every
// field; probeCounts holds LightHeaderCount(), AvailableLightCount() and
// TvLightCount(); probeActive holds StaticLightCount(), StaticLightIndex(v),
// DynamicLightCount() and DynamicLightIndex(v).
std::string BuildLightProbeSource(std::string& error)
{
    const char* const file = "glob.vert";
    const std::string source = CaneEmbeddedShader(file);
    std::string probe = std::string("#version ") + kShaderVersionGL41 + "\n";

    const std::string maxLights = ExtractDefine(source, "MAX_LIGHTS");
    if (maxLights.empty())
        error += "glob.vert has no MAX_LIGHTS; ";
    probe += maxLights;
    const auto addRegion = [&](const char* name)
    {
        const std::string region = ExtractProbeRegion(source, name);
        if (region.empty())
            error += std::string(file) + " has no probe region " + name + "; ";
        probe += region;
    };
    const auto addSampler = [&](const char* name)
    {
        const std::string declaration = ExtractUniformDeclaration(source, name);
        if (declaration.empty())
            error += std::string(file) + " declares no sampler " + name + "; ";
        probe += declaration;
    };

    addRegion("LIGHT");
    addSampler("lightTex");
    addSampler("activeLightTex");
    addSampler("tvLightTex");
    for (const char* name : { "FetchLightAt", "FetchLight", "FetchTvLight", "LightHeaderCount",
        "AvailableLightCount", "TvLightCount", "StaticLightCount", "StaticLightIndex", "DynamicLightCount",
        "DynamicLightIndex" })
        addRegion(name);

    probe += R"(
uniform int probeTv;
uniform int probeRecords;
flat out ivec4 probeFlags;
flat out uvec4 probeTexel1;
flat out uvec4 probeTexel2;
flat out uvec4 probeTexel3;
flat out uvec4 probeTexel4;
flat out uvec4 probeTexel5;
flat out uvec4 probeTexel6;
flat out uvec4 probeTexel7;
flat out uvec4 probeTexel8;
flat out uvec4 probeTexel9;
flat out uvec4 probeTexel10;
flat out uvec4 probeTexel11;
flat out uvec4 probeTexel12;
flat out ivec4 probeCounts;
flat out ivec4 probeActive;

void main()
{
    int record = gl_VertexID % max(probeRecords, 1);
    LIGHT lt = probeTv != 0 ? FetchTvLight(record) : FetchLight(record);
    probeFlags = ivec4(lt.lightk, lt.fExcludeDynamicObjects, lt.fActive, lt.fDynamic);
    probeTexel1 = floatBitsToUint(lt.pos);
    probeTexel2 = floatBitsToUint(lt.dir);
    probeTexel3 = floatBitsToUint(lt.color);
    probeTexel4 = floatBitsToUint(vec4(lt.constant, lt.invDst, lt.pad4, lt.dst));
    probeTexel5 = floatBitsToUint(lt.ru);
    probeTexel6 = floatBitsToUint(lt.du);
    probeTexel7 = floatBitsToUint(lt.matFrustrum[0]);
    probeTexel8 = floatBitsToUint(lt.matFrustrum[1]);
    probeTexel9 = floatBitsToUint(lt.matFrustrum[2]);
    probeTexel10 = floatBitsToUint(lt.matFrustrum[3]);
    probeTexel11 = floatBitsToUint(lt.falloffScale);
    probeTexel12 = floatBitsToUint(lt.falloffBias);
    probeCounts = ivec4(LightHeaderCount(), AvailableLightCount(), TvLightCount(), 0);
    probeActive = ivec4(StaticLightCount(), StaticLightIndex(gl_VertexID),
        DynamicLightCount(), DynamicLightIndex(gl_VertexID));
}
)";
    return probe;
}

// Uploads `lights` the way light.cpp's AllocateLightBlkList does: the store,
// its texture, then the header (when `withHeader`) and the LIGHTBLK array.
// `extraTexels` appends a partial record the counts must ignore.
GLuint CreateLightBuffer(bool withHeader, int headerCount, const std::vector<LightBlk>& lights, int extraTexels,
    GLenum format)
{
    const GLsizeiptr headerSize = withHeader ? static_cast<GLsizeiptr>(sizeof(LightSsboHeader)) : 0;
    const GLsizeiptr lightsSize = static_cast<GLsizeiptr>(sizeof(LightBlk) * lights.size());
    const GLsizeiptr extraSize = static_cast<GLsizeiptr>(extraTexels) * 16;
    std::vector<unsigned char> extra(static_cast<size_t>(extraSize), 0x5A);

    GLuint buffer = 0;
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_TEXTURE_BUFFER, buffer);
    glBufferData(GL_TEXTURE_BUFFER, headerSize + lightsSize + extraSize, nullptr, GL_DYNAMIC_DRAW);
    CHECK(CreateBufferTexture(buffer, format) != 0);
    if (withHeader)
    {
        LightSsboHeader header = {};
        header.numLights = headerCount;
        glBufferSubData(GL_TEXTURE_BUFFER, 0, headerSize, &header);
    }
    glBufferSubData(GL_TEXTURE_BUFFER, headerSize, lightsSize, lights.data());
    if (extraSize > 0)
        glBufferSubData(GL_TEXTURE_BUFFER, headerSize + lightsSize, extraSize, extra.data());
    glBindBuffer(GL_TEXTURE_BUFFER, 0);
    return buffer;
}

GLuint CreateActiveLightBuffer(const ActiveLights& active)
{
    GLuint buffer = 0;
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_TEXTURE_BUFFER, buffer);
    glBufferData(GL_TEXTURE_BUFFER, sizeof(ActiveLights), &active, GL_DYNAMIC_DRAW);
    CHECK(CreateBufferTexture(buffer, kActiveLightTextureFormat) != 0);
    glBindBuffer(GL_TEXTURE_BUFFER, 0);
    return buffer;
}

using LightCapture = std::array<uint32_t, kLightProbeVaryingCount * 4>;

// Draws `vertexCount` points with the textures bound the way light.cpp's
// BindSwLightTextures binds them and returns the captured words.
std::vector<LightCapture> RunLightProbe(GLuint program, int vertexCount, bool tv, int records,
    GLuint lightBuffer, GLuint activeBuffer, GLuint tvBuffer)
{
    glActiveTexture(GL_TEXTURE0 + kLightTextureUnit);
    glBindTexture(GL_TEXTURE_BUFFER, GetBufferTexture(lightBuffer));
    glActiveTexture(GL_TEXTURE0 + kActiveLightTextureUnit);
    glBindTexture(GL_TEXTURE_BUFFER, GetBufferTexture(activeBuffer));
    glActiveTexture(GL_TEXTURE0 + kTvLightTextureUnit);
    glBindTexture(GL_TEXTURE_BUFFER, GetBufferTexture(tvBuffer));
    glActiveTexture(GL_TEXTURE0);

    glUniform1i(glGetUniformLocation(program, "probeTv"), tv ? 1 : 0);
    glUniform1i(glGetUniformLocation(program, "probeRecords"), records);

    GLuint vao = 0;
    GLuint capture = 0;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    const GLsizeiptr captureSize = static_cast<GLsizeiptr>(vertexCount * sizeof(LightCapture));
    glGenBuffers(1, &capture);
    glBindBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, capture);
    glBufferData(GL_TRANSFORM_FEEDBACK_BUFFER, captureSize, nullptr, GL_STATIC_READ);
    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, capture);

    glEnable(GL_RASTERIZER_DISCARD);
    glBeginTransformFeedback(GL_POINTS);
    glDrawArrays(GL_POINTS, 0, vertexCount);
    glEndTransformFeedback();
    glDisable(GL_RASTERIZER_DISCARD);

    LightCapture filler;
    filler.fill(0xDEADBEEFu);
    std::vector<LightCapture> captured(static_cast<size_t>(vertexCount), filler);
    glBindBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, capture);
    glGetBufferSubData(GL_TRANSFORM_FEEDBACK_BUFFER, 0, captureSize, captured.data());

    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, 0);
    glBindVertexArray(0);
    glDeleteBuffers(1, &capture);
    glDeleteVertexArrays(1, &vao);
    for (GLint unit : { kLightTextureUnit, kActiveLightTextureUnit, kTvLightTextureUnit })
    {
        glActiveTexture(GL_TEXTURE0 + unit);
        glBindTexture(GL_TEXTURE_BUFFER, 0);
    }
    glActiveTexture(GL_TEXTURE0);
    return captured;
}

// Four lights, one per LIGHTK, with mixed flags; TV lights reuse the same
// generator with a different seed.
std::vector<LightBlk> MakeProbeLights(int count, float seed)
{
    std::vector<LightBlk> lights(static_cast<size_t>(count));
    struct Ints { int lightk, fExclude, fActive, fDynamic; };
    const Ints ints[4] =
    {
        { 0, 0, 1, 0 },
        { 1, 1, 0, 1 },
        { 2, 1, 1, 0 },
        { 3, 0, 0, 1 },
    };
    for (int i = 0; i < count; ++i)
    {
        LightBlk& light = lights[static_cast<size_t>(i)];
        const float f = static_cast<float>(i + 1) * seed;
        light.lightk = ints[i % 4].lightk;
        light.fExcludeDynamicObjects = ints[i % 4].fExclude;
        light.fActive = ints[i % 4].fActive;
        light.fDynamic = ints[i % 4].fDynamic;
        light.pos = glm::vec4(-120.5f * f, 33.25f, -0.0f, 1.0f);
        light.dir = glm::vec4(glm::normalize(glm::vec3(0.2f * f, -0.9f, 0.4f)), -0.0f);
        light.color = glm::vec4(0.75f / f, -0.5f, 1.5f * f, 0.25f);
        light.constant = -0.375f * f;
        light.invDst = 812.5f / f;
        light.pad4 = -1e-3f * f;
        light.dst = 2500.0f + f;
        light.ru = glm::vec4(-0.25f * f, 0.5f, -1.75f, 3e-7f);
        light.du = glm::vec4(0.125f, -0.0625f * f, 2.0f, -9.5f);
        // Non-symmetric so a transposed or column-swapped read shows.
        light.matFrustrum = glm::perspective(glm::radians(35.0f + 10.0f * static_cast<float>(i % 8)), 1.25f + 0.1f * static_cast<float>(i % 8), 0.5f, 900.0f) *
            glm::lookAt(glm::vec3(-4.0f * f, 12.0f, 3.0f + static_cast<float>(i)), glm::vec3(1.0f, 0.0f, -2.0f), glm::vec3(0.0f, 0.0f, 1.0f)) +
            glm::mat4(glm::vec4(0.0f, 0.01f, 0.02f, 0.03f), glm::vec4(0.1f, 0.11f, 0.12f, 0.13f),
                      glm::vec4(0.2f, 0.21f, 0.22f, 0.23f), glm::vec4(0.3f, 0.31f, 0.32f, 0.33f)) * f;
        light.falloffScale = glm::vec4(-2.5f * f, 4.0f, -0.0f, 7.25f);
        light.falloffBias = glm::vec4(1.0f, -3.5f / f, 0.5f, -12.0f * f);
    }
    return lights;
}

// Returns how many records mismatched.
int CompareLights(const char* label, const std::vector<LightCapture>& captured, const std::vector<LightBlk>& lights,
    int vertexCount)
{
    int badRecords = 0;
    const int records = static_cast<int>(lights.size());
    for (int vertex = 0; vertex < vertexCount; ++vertex)
    {
        const std::array<uint32_t, kLightWords> expected = LightWords(lights[static_cast<size_t>(vertex % records)]);
        int mismatches = 0;
        for (int texel = 0; texel < kLightTexels; ++texel)
        {
            const uint32_t* got = &captured[static_cast<size_t>(vertex)][texel * 4];
            const uint32_t* want = &expected[texel * 4];
            if (std::memcmp(got, want, 16) == 0)
                continue;
            ++mismatches;
            if (vertex < records)
                std::printf("  MISMATCH %s record %d texel %d %s: got %08x %08x %08x %08x want %08x %08x %08x %08x\n",
                    label, vertex, texel, kLightTexelNames[texel],
                    got[0], got[1], got[2], got[3], want[0], want[1], want[2], want[3]);
        }
        CHECK(mismatches == 0);
        badRecords += mismatches != 0 ? 1 : 0;
    }
    return badRecords;
}

void TestGlobLightFetch()
{
    std::printf("glob light fetch helpers from glob.vert (transform feedback vs C++ layout):\n");
    std::string error;
    const std::string source = BuildLightProbeSource(error);
    if (!error.empty())
    {
        std::fprintf(stderr, "FAIL: %s\n", error.c_str());
        CHECK(error.empty());
        return;
    }
    const GLuint program = LinkStages(CompileSource(GL_VERTEX_SHADER, "glob.vert", source),
        CompileSource(GL_FRAGMENT_SHADER, "probe.frag", std::string("#version ") + kShaderVersionGL41 + "\nvoid main() {}\n"),
        error, CaptureLightProbe);
    if (program == 0)
    {
        std::fprintf(stderr, "FAIL: glob.vert light probe did not build: %s\n", error.c_str());
        CHECK(program != 0);
        return;
    }
    CHECK(CheckSamplerUnits("glob.vert lights", program) == 3);
    // Records mix ints and floats, so they must be read as integers; see kLightTextureFormat.
    const GLenum lightTexType = UniformType(program, "lightTex");
    const GLenum activeTexType = UniformType(program, "activeLightTex");
    const GLenum tvTexType = UniformType(program, "tvLightTex");
    std::printf("  lightTex type 0x%04x, tvLightTex type 0x%04x (want 0x%04x), activeLightTex type 0x%04x (want 0x%04x)\n",
        lightTexType, tvTexType, GL_UNSIGNED_INT_SAMPLER_BUFFER, activeTexType, GL_INT_SAMPLER_BUFFER);
    CHECK(lightTexType == GL_UNSIGNED_INT_SAMPLER_BUFFER);
    CHECK(tvTexType == GL_UNSIGNED_INT_SAMPLER_BUFFER);
    CHECK(activeTexType == GL_INT_SAMPLER_BUFFER);

    glUseProgram(program);

    // ---- main lights: header count against the stored records
    const std::vector<LightBlk> lights = MakeProbeLights(4, 1.0f);
    const int recordCount = static_cast<int>(lights.size());
    ActiveLights active = {};
    for (int i = 0; i < kMaxLights; ++i)
    {
        active.staticLightIndices[i] = 1000 + i;
        active.dynamicLightIndices[i] = -2000 - i;
    }
    active.numStaticLights = 2;
    active.numDynamicLights = 3;
    const GLuint baseActive = CreateActiveLightBuffer(active);

    struct CountCase
    {
        const char* name;
        int header;
        int extraTexels;
        int expected;
    };
    const CountCase kCountCases[] =
    {
        { "header == records", recordCount, 0, recordCount },
        { "header above records", recordCount + 4, 0, recordCount },
        { "header below records", recordCount - 1, 0, recordCount - 1 },
        { "header zero", 0, 0, 0 },
        { "header negative", -4, 0, 0 },
        { "header above MAX_LIGHTS", 100000, 0, recordCount },
        { "partial trailing record", recordCount + 6, kLightTexels - 1, recordCount },
    };
    for (const CountCase& countCase : kCountCases)
    {
        const GLuint lightBuffer = CreateLightBuffer(true, countCase.header, lights, countCase.extraTexels, kLightTextureFormat);
        const auto captured = RunLightProbe(program, recordCount, false, recordCount, lightBuffer, baseActive, 0);
        const int bad = CompareLights(countCase.name, captured, lights, recordCount);
        const int header = AsInt(captured[0][13 * 4 + 0]);
        const int available = AsInt(captured[0][13 * 4 + 1]);
        std::printf("  %s %s: records %d/%d bit-exact, LightHeaderCount() %d, AvailableLightCount() %d (expected %d, %d)\n",
            bad == 0 && header == countCase.header && available == countCase.expected ? "ok" : "MISMATCH",
            countCase.name, recordCount - bad, recordCount, header, available, countCase.header, countCase.expected);
        CHECK(header == countCase.header);
        CHECK(available == countCase.expected);
        DeleteBufferWithTexture(lightBuffer);
    }

    // More records than MAX_LIGHTS: AvailableLightCount() stops at MAX_LIGHTS.
    {
        const std::vector<LightBlk> many = MakeProbeLights(kMaxLights + 4, 0.5f);
        const int manyCount = static_cast<int>(many.size());
        const GLuint manyBuffer = CreateLightBuffer(true, manyCount, many, 0, kLightTextureFormat);
        const auto captured = RunLightProbe(program, manyCount, false, manyCount, manyBuffer, baseActive, 0);
        const int bad = CompareLights("records above MAX_LIGHTS", captured, many, manyCount);
        const int available = AsInt(captured[0][13 * 4 + 1]);
        std::printf("  %s records above MAX_LIGHTS: records %d/%d bit-exact, AvailableLightCount() %d (expected %d)\n",
            bad == 0 && available == kMaxLights ? "ok" : "MISMATCH", manyCount - bad, manyCount, available, kMaxLights);
        CHECK(available == kMaxLights);
        DeleteBufferWithTexture(manyBuffer);
    }
    DeleteBufferWithTexture(baseActive);

    // ---- active lists: every index slot, at counts 0, 1 and near 256
    struct ActiveCase
    {
        const char* name;
        int numStatic;
        int numDynamic;
    };
    const ActiveCase kActiveCases[] =
    {
        { "no active lights", 0, 0 },
        { "one static, one dynamic", 1, 1 },
        { "255 static, 256 dynamic", 255, 256 },
        { "256 static, 254 dynamic", 256, 254 },
    };
    const GLuint lightBuffer = CreateLightBuffer(true, recordCount, lights, 0, kLightTextureFormat);
    for (const ActiveCase& activeCase : kActiveCases)
    {
        active.numStaticLights = activeCase.numStatic;
        active.numDynamicLights = activeCase.numDynamic;
        const GLuint activeBuffer = CreateActiveLightBuffer(active);
        const auto captured = RunLightProbe(program, kMaxLights, false, recordCount, lightBuffer, activeBuffer, 0);
        CompareLights(activeCase.name, captured, lights, kMaxLights);
        int wrongIndices = 0;
        for (int i = 0; i < kMaxLights; ++i)
        {
            const uint32_t* got = &captured[static_cast<size_t>(i)][14 * 4];
            const bool ok = AsInt(got[0]) == activeCase.numStatic && AsInt(got[1]) == active.staticLightIndices[i] &&
                AsInt(got[2]) == activeCase.numDynamic && AsInt(got[3]) == active.dynamicLightIndices[i];
            if (!ok && wrongIndices++ < 4)
                std::printf("  MISMATCH %s slot %d: counts %d/%d, indices %d/%d (want %d/%d, %d/%d)\n",
                    activeCase.name, i, AsInt(got[0]), AsInt(got[2]), AsInt(got[1]), AsInt(got[3]),
                    activeCase.numStatic, activeCase.numDynamic, active.staticLightIndices[i], active.dynamicLightIndices[i]);
            CHECK(ok);
        }
        std::printf("  %s %s: StaticLightCount() / DynamicLightCount() and all %d index slots of each list\n",
            wrongIndices == 0 ? "ok" : "MISMATCH", activeCase.name, kMaxLights);
        DeleteBufferWithTexture(activeBuffer);
    }
    DeleteBufferWithTexture(lightBuffer);

    // ---- TV lights: two records, no header
    const std::vector<LightBlk> tvLights = MakeProbeLights(2, -1.5f);
    for (int extraTexels : { 0, kLightTexels - 1 })
    {
        const GLuint tvBuffer = CreateLightBuffer(false, 0, tvLights, extraTexels, kTvLightTextureFormat);
        const auto captured = RunLightProbe(program, 2, true, 2, 0, 0, tvBuffer);
        const int bad = CompareLights("tv", captured, tvLights, 2);
        const int tvCount = AsInt(captured[0][13 * 4 + 2]);
        std::printf("  %s TV lights (+%d texels): records %d/2 bit-exact, TvLightCount() %d (expected 2)\n",
            bad == 0 && tvCount == 2 ? "ok" : "MISMATCH", extraTexels, 2 - bad, tvCount);
        CHECK(tvCount == 2);
        DeleteBufferWithTexture(tvBuffer);
    }

    // With nothing bound every count is zero, so glob skips lighting.
    const auto unbound = RunLightProbe(program, 1, false, 1, 0, 0, 0);
    const uint32_t* counts = &unbound[0][13 * 4];
    const uint32_t* lists = &unbound[0][14 * 4];
    std::printf("  unbound textures: AvailableLightCount() %d, TvLightCount() %d, StaticLightCount() %d, DynamicLightCount() %d\n",
        AsInt(counts[1]), AsInt(counts[2]), AsInt(lists[0]), AsInt(lists[2]));
    CHECK(AsInt(counts[1]) == 0);
    CHECK(AsInt(counts[2]) == 0);

    const GLenum glError = glGetError();
    if (glError != GL_NO_ERROR)
        std::fprintf(stderr, "glob.vert light probe raised GL error 0x%04x\n", glError);
    CHECK(glError == GL_NO_ERROR);

    glUseProgram(0);
    CHECK(BufferTextures().empty());
    glDeleteProgram(program);
}

// ---- whole glob program -----------------------------------------------------

// Removes // and /* */ comments, keeping line breaks.
std::string StripComments(const std::string& source)
{
    std::string text;
    size_t i = 0;
    while (i < source.size())
    {
        if (source.compare(i, 2, "//") == 0)
        {
            i = source.find('\n', i);
            if (i == std::string::npos)
                break;
        }
        else if (source.compare(i, 2, "/*") == 0)
        {
            const size_t end = source.find("*/", i + 2);
            const size_t stop = end == std::string::npos ? source.size() : end + 2;
            text.append(static_cast<size_t>(std::count(source.begin() + i, source.begin() + stop, '\n')), '\n');
            i = stop;
        }
        else
        {
            text += source[i++];
        }
    }
    return text;
}

// GLSL 4.10 has no buffer blocks; glob.vert's last one, CACHEDLIGHTING, is
// gone, so this now only guards against one coming back. Replaces each "layout(std430, binding = N) [readonly] buffer
// NAME { T member[]; };" with a fixed array of its member, a uniform when
// readonly; the size is arbitrary since the tests never reach the stubbed
// reads. Fills `error` if a buffer block is left. With none left the source
// comes back unchanged.
std::string StubBufferBlocks(const std::string& source, std::vector<std::string>& stubbed, std::string& error)
{
    const std::regex block(R"(layout\s*\(\s*std430\s*,\s*binding\s*=\s*\d+\s*\)\s*(readonly\s+)?buffer\s+(\w+)\s*)"
        R"(\{\s*(\w+)\s+(\w+)\s*\[\s*\]\s*;\s*\}\s*;)");
    std::string text;
    size_t copied = 0;
    for (auto match = std::sregex_iterator(source.begin(), source.end(), block); match != std::sregex_iterator(); ++match)
    {
        text.append(source, copied, static_cast<size_t>(match->position()) - copied);
        text += std::string((*match)[1].matched ? "uniform " : "") + (*match)[3].str() + " " + (*match)[4].str() +
            "[8]; // stub for " + (*match)[2].str();
        copied = static_cast<size_t>(match->position() + match->length());
        stubbed.push_back((*match)[2].str());
    }
    text.append(source, copied, std::string::npos);

    std::istringstream lines(StripComments(text));
    const std::regex leftover(R"(\bbuffer\b|\bstd430\b)");
    int lineNumber = 0;
    for (std::string line; std::getline(lines, line);)
    {
        ++lineNumber;
        if (std::regex_search(line, leftover))
            error += "line " + std::to_string(lineNumber) + " still declares a buffer block: " + line + "; ";
    }
    return text;
}

// glob.vert's MATERIAL as the cache and kGlobFeedbackVaryings lay it out.
struct CachedMaterial
{
    float ambient;
    float pad[3];
    glm::vec4 midtone;
    glm::vec4 light;
};
static_assert(sizeof(CachedMaterial) == kCachedMaterialStride, "one cache record per vertex");
static_assert(offsetof(CachedMaterial, midtone) == 16, "texel 3 * id + 1");
static_assert(offsetof(CachedMaterial, light) == 32, "texel 3 * id + 2");

// The varyings gl.cpp sets on glGlobShader before it links.
void CaptureGlobMaterial(GLuint program)
{
    glTransformFeedbackVaryings(program, kGlobFeedbackVaryingCount, kGlobFeedbackVaryings, GL_INTERLEAVED_ATTRIBS);
}

// The game's glob program as it links on 4.1: the unmodified sources with the
// game's transform feedback varyings. Apple's driver rejects some valid
// programs at link with no error in the info log, e.g. one with `continue`
// inside a switch inside a loop.
void TestGlobProgramLinks()
{
    std::printf("glob program (unmodified glob.vert and glob.frag, cache varyings):\n");
    std::vector<std::string> stubbed;
    std::string error;
    std::string source;
    try
    {
        source = StubBufferBlocks(CaneEmbeddedShader("glob.vert"), stubbed, error);
    }
    catch (const std::exception& exception)
    {
        error = exception.what();
    }
    std::string names;
    for (const std::string& name : stubbed)
        names += " " + name;
    std::printf("  stubbed %zu buffer blocks:%s\n", stubbed.size(), names.c_str());
    // CACHEDLIGHTING was the last one; there is nothing left to stub.
    CHECK(stubbed.empty());
    if (!error.empty())
    {
        std::fprintf(stderr, "FAIL: glob.vert buffer blocks could not be stubbed: %s\n", error.c_str());
        CHECK(error.empty());
        return;
    }

    StageResult vertex = CompileSource(GL_VERTEX_SHADER, "glob.vert",
        RewriteShaderVersion(source, kShaderVersionGL41));
    StageResult fragment = CompileStage(GL_FRAGMENT_SHADER, "glob.frag");
    if (vertex.shader == 0 || fragment.shader == 0)
    {
        std::fprintf(stderr, "FAIL: glob did not compile: %s\n",
            (!vertex.error.empty() ? vertex.error : fragment.error).c_str());
        if (vertex.shader != 0) glDeleteShader(vertex.shader);
        if (fragment.shader != 0) glDeleteShader(fragment.shader);
        CHECK(vertex.shader != 0 && fragment.shader != 0);
        return;
    }

    // Linked here rather than through LinkStages to keep the whole info log.
    const GLuint program = glCreateProgram();
    glAttachShader(program, vertex.shader);
    glAttachShader(program, fragment.shader);
    CaptureGlobMaterial(program);
    glLinkProgram(program);
    glDeleteShader(vertex.shader);
    glDeleteShader(fragment.shader);
    GLint linked = GL_FALSE;
    glGetProgramiv(program, GL_LINK_STATUS, &linked);
    GLint length = 0;
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &length);
    std::string log(static_cast<size_t>(length > 1 ? length : 1), '\0');
    glGetProgramInfoLog(program, length, nullptr, log.data());
    log.resize(std::strlen(log.c_str()));
    if (linked == GL_FALSE)
        std::fprintf(stderr, "FAIL: glob did not link; info log: %s\n", log.empty() ? "(empty)" : log.c_str());
    else
        std::printf("  ok: glob compiled and linked; info log: %s\n", log.empty() ? "(empty)" : log.c_str());
    CHECK(linked == GL_TRUE);
    if (linked == GL_TRUE)
    {
        GLint mode = 0;
        GLint count = 0;
        glGetProgramiv(program, GL_TRANSFORM_FEEDBACK_BUFFER_MODE, &mode);
        glGetProgramiv(program, GL_TRANSFORM_FEEDBACK_VARYINGS, &count);
        std::printf("  transform feedback: %d varyings, mode 0x%04x\n", count, mode);
        CHECK(mode == GL_INTERLEAVED_ATTRIBS);
        CHECK(count == kGlobFeedbackVaryingCount);
    }
    glDeleteProgram(program);
}

// ---- whole glob vertex shader -----------------------------------------------

// Mirrors gl.h's ROGL (the RO block), which this GL-only test can't include.
struct alignas(16) RoGl
{
    glm::mat4 model;
    float uAlpha;
    float uFog;
    float darken;
    int32_t grfglob;
    int32_t blotTvLight;
    int32_t warpType;
    int32_t warpCmat;
    int32_t warpCvtx;
    glm::mat4 amatDpos[4];
    glm::mat4 amatDuv[4];
    int32_t fDynamic;
    float sRadius;
    int32_t fDynamicLight;
    int32_t trlk;
    glm::vec4 posCenter;
};
static_assert(sizeof(RoGl) == 640, "ROGL is 640 bytes");
static_assert(offsetof(RoGl, warpType) == 84, "ROGL warpType");
static_assert(offsetof(RoGl, amatDpos) == 96, "ROGL amatDpos");
static_assert(offsetof(RoGl, amatDuv) == 352, "ROGL amatDuv");
static_assert(offsetof(RoGl, posCenter) == 624, "ROGL posCenter");

// Mirrors glob.h's VERTICE and the attribute setup in glob.cpp.
struct GlobVertex
{
    glm::vec3 pos;
    glm::vec3 normal;
    glm::vec4 color;
    glm::vec2 uv;
    glm::uvec4 boneIndices;
    glm::vec4 boneWeights;
};

constexpr int kWarpNone = 0;
constexpr int kWarpPos = 1;
constexpr int kWarpUv = 2;
constexpr int kWarpBoth = 3;

// The per-object data one draw sees, as CPU arrays.
struct GlobData
{
    std::vector<glm::mat4> bones;
    std::vector<glm::vec4> poseDpos;
    std::vector<glm::vec4> poseDnormal;
    std::vector<float> poseWeights;
    std::vector<glm::vec4> warpState;
};

struct GlobCase
{
    const char* name;
    bool skin;
    bool pose;
    int warpType;
    bool animateUv;
    const GlobData* data;
};

struct GlobScene
{
    std::vector<GlobVertex> vertices;
    glm::mat4 model;
    glm::mat4 worldToClip;
    int poseCount;
    int warpCmat;
    int warpCvtx;
    glm::mat4 amatDpos[4];
    glm::mat4 amatDuv[4];
    glm::vec2 uvOffsets;
};

// gl_Position, worldNormal and texcoord, interleaved without padding.
constexpr int kGlobCaptureFloats = 9;
using GlobCapture = std::array<float, kGlobCaptureFloats>;

void CaptureGlobVertex(GLuint program)
{
    const char* const varyings[] = { "gl_Position", "worldNormal", "texcoord" };
    glTransformFeedbackVaryings(program, 3, varyings, GL_INTERLEAVED_ATTRIBS);
}

glm::vec3 SafeNormalizeRef(const glm::vec3& value)
{
    const float length2 = glm::dot(value, value);
    return length2 > 1e-12f ? value * (1.0f / std::sqrt(length2)) : glm::vec3(0.0f);
}

// Mirrors glob.vert's main, ApplyPoses and ApplyWarp for one vertex with
// three-way lighting and fog off.
GlobCapture ReferenceGlobVertex(const GlobScene& scene, const GlobCase& c, int vertexId)
{
    const GlobVertex& vertex = scene.vertices[static_cast<size_t>(vertexId)];
    const GlobData& data = *c.data;
    glm::vec4 position(vertex.pos, 1.0f);
    glm::vec3 normal = vertex.normal;

    if (c.pose)
    {
        glm::vec3 dpos(0.0f);
        glm::vec3 dnormal(0.0f);
        const int count = std::min(std::max(scene.poseCount, 0), static_cast<int>(data.poseWeights.size()));
        const int poseBase = vertexId * scene.poseCount;
        for (int ipose = 0; ipose < count; ++ipose)
        {
            const float weight = data.poseWeights[static_cast<size_t>(ipose)];
            if (weight == 0.0f)
                continue;
            const int poseIndex = poseBase + ipose;
            if (poseIndex < 0 || poseIndex >= static_cast<int>(data.poseDpos.size()) ||
                poseIndex >= static_cast<int>(data.poseDnormal.size()))
                continue;
            dpos += glm::vec3(data.poseDpos[static_cast<size_t>(poseIndex)]) * weight;
            dnormal += glm::vec3(data.poseDnormal[static_cast<size_t>(poseIndex)]) * weight;
        }
        position = glm::vec4(glm::vec3(position) + dpos, position.w);
        normal += dnormal;
        const float normalLength2 = glm::dot(normal, normal);
        if (normalLength2 > 1e-8f)
            normal *= 1.0f / std::sqrt(normalLength2);
    }

    if (c.skin)
    {
        glm::mat4 matSkin(0.0f);
        float totalWeight = 0.0f;
        for (int i = 0; i < 4; ++i)
        {
            if (vertex.boneWeights[i] != 0.0f && vertex.boneIndices[i] < data.bones.size())
            {
                matSkin += data.bones[vertex.boneIndices[i]] * vertex.boneWeights[i];
                totalWeight += vertex.boneWeights[i];
            }
        }
        if (totalWeight > 0.0f)
        {
            position = matSkin * position;
            const glm::vec3 skinnedNormal = glm::mat3(matSkin) * normal;
            const float normalLength2 = glm::dot(skinnedNormal, skinnedNormal);
            normal = normalLength2 > 1e-8f ? skinnedNormal * (1.0f / std::sqrt(normalLength2)) : glm::vec3(0.0f);
        }
    }

    glm::vec4 worldPos = scene.model * position;
    const glm::vec3 worldNormal = SafeNormalizeRef(glm::mat3(scene.model) * normal);
    glm::vec2 uv = vertex.uv;
    if (c.animateUv)
        uv += scene.uvOffsets;

    const int matrixCount = std::clamp(scene.warpCmat, 0, 4);
    if (c.warpType != kWarpNone && scene.warpCvtx > 0)
    {
        glm::vec4 sumPos(0.0f);
        glm::vec2 sumUv(0.0f);
        for (int imat = 0; imat < matrixCount; ++imat)
        {
            const int stateIndex = imat * scene.warpCvtx + vertexId;
            if (stateIndex < 0 || stateIndex >= static_cast<int>(data.warpState.size()))
                continue;
            const glm::vec4 st = data.warpState[static_cast<size_t>(stateIndex)];
            sumPos += scene.amatDpos[imat] * st;
            sumUv += glm::vec2(scene.amatDuv[imat] * st);
        }
        if (c.warpType == kWarpPos || c.warpType == kWarpBoth)
            worldPos = glm::vec4(glm::vec3(worldPos) + glm::vec3(sumPos), worldPos.w);
        if (c.warpType == kWarpUv || c.warpType == kWarpBoth)
            uv += sumUv;
    }

    const glm::vec4 clip = scene.worldToClip * worldPos;
    return { clip.x, clip.y, clip.z, clip.w, worldNormal.x, worldNormal.y, worldNormal.z, uv.x, uv.y };
}

bool NearlyEqualCapture(const GlobCapture& actual, const GlobCapture& expected)
{
    for (int i = 0; i < kGlobCaptureFloats; ++i)
    {
        const float tolerance = 1e-4f * std::fmax(1.0f, std::fabs(expected[i]));
        if (!(std::fabs(actual[i] - expected[i]) <= tolerance))
            return false;
    }
    return true;
}

// The buffers one GlobData uploads, built the way the game builds them.
struct GlobBuffers
{
    GLuint boneBuffer = 0;
    GLuint boneTexture = 0;
    GLuint poseDpos = 0;
    GLuint poseDnormal = 0;
    GLuint poseWeights = 0;
    GLuint warpState = 0;
};

GlobBuffers UploadGlobData(const GlobData& data)
{
    GlobBuffers buffers;
    // GLOBSET::boneMatrixTexture: built by hand in glob.cpp, not in the registry.
    glGenBuffers(1, &buffers.boneBuffer);
    glBindBuffer(GL_TEXTURE_BUFFER, buffers.boneBuffer);
    glBufferData(GL_TEXTURE_BUFFER, static_cast<GLsizeiptr>(data.bones.size() * sizeof(glm::mat4)),
        data.bones.data(), GL_DYNAMIC_DRAW);
    glGenTextures(1, &buffers.boneTexture);
    glBindTexture(GL_TEXTURE_BUFFER, buffers.boneTexture);
    glTexBuffer(GL_TEXTURE_BUFFER, GL_RGBA32F, buffers.boneBuffer);
    glBindTexture(GL_TEXTURE_BUFFER, 0);
    glBindBuffer(GL_TEXTURE_BUFFER, 0);

    // GLOB::poseDposSSBO / poseDnormalSSBO and the pose-weight sidecar.
    buffers.poseDpos = CreateBufferWithTexture(data.poseDpos, kPoseDposTextureFormat);
    buffers.poseDnormal = CreateBufferWithTexture(data.poseDnormal, kPoseDnormalTextureFormat);
    buffers.poseWeights = CreateBufferWithTexture(data.poseWeights, kPoseWeightTextureFormat);

    // WRBGLOB_GL::ssboState: ApplyWrGlob allocates the store and views it,
    // then UpdateWrStateVectors fills it with glBufferSubData.
    const GLsizeiptr warpBytes = static_cast<GLsizeiptr>(data.warpState.size() * sizeof(glm::vec4));
    glGenBuffers(1, &buffers.warpState);
    glBindBuffer(GL_TEXTURE_BUFFER, buffers.warpState);
    glBufferData(GL_TEXTURE_BUFFER, warpBytes, nullptr, GL_DYNAMIC_DRAW);
    CHECK(CreateBufferTexture(buffers.warpState, kWarpStateTextureFormat) != 0);
    glBindBuffer(GL_TEXTURE_BUFFER, buffers.warpState);
    glBufferSubData(GL_TEXTURE_BUFFER, 0, warpBytes, data.warpState.data());
    glBindBuffer(GL_TEXTURE_BUFFER, 0);
    return buffers;
}

void DeleteGlobBuffers(const GlobBuffers& buffers)
{
    glDeleteTextures(1, &buffers.boneTexture);
    glDeleteBuffers(1, &buffers.boneBuffer);
    DeleteBufferWithTexture(buffers.poseDpos);
    DeleteBufferWithTexture(buffers.poseDnormal);
    DeleteBufferWithTexture(buffers.poseWeights);
    DeleteBufferWithTexture(buffers.warpState);
}

// Draws every vertex as a point with the textures bound on the units
// render.cpp's DrawGlob uses, capturing gl_Position, worldNormal and texcoord.
// A null `buffers` leaves all five units unbound.
std::vector<GlobCapture> RunGlob(GLuint program, GLuint roBuffer, const GlobScene& scene, const GlobCase& c,
    const GlobBuffers* buffers)
{
    RoGl ro{};
    ro.model = scene.model;
    ro.uAlpha = 1.0f;
    ro.warpType = c.warpType;
    ro.warpCmat = scene.warpCmat;
    ro.warpCvtx = scene.warpCvtx;
    for (int i = 0; i < 4; ++i)
    {
        ro.amatDpos[i] = scene.amatDpos[i];
        ro.amatDuv[i] = scene.amatDuv[i];
    }
    ro.trlk = 2; // TRLK_Dynamic; three-way lighting is off (rko 0) anyway
    glBindBuffer(GL_UNIFORM_BUFFER, roBuffer);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(ro), &ro);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    glUniform1i(glGetUniformLocation(program, "fSkin"), c.skin ? 1 : 0);
    glUniform1i(glGetUniformLocation(program, "fPose"), c.pose ? 1 : 0);
    glUniform1i(glGetUniformLocation(program, "poseCount"), scene.poseCount);
    glUniform1i(glGetUniformLocation(program, "rko"), 0);
    glUniform1i(glGetUniformLocation(program, "fAnimateUv"), c.animateUv ? 1 : 0);
    glUniform2f(glGetUniformLocation(program, "uvOffsets"), scene.uvOffsets.x, scene.uvOffsets.y);

    const auto bind = [](GLint unit, GLuint texture)
    {
        glActiveTexture(GL_TEXTURE0 + unit);
        glBindTexture(GL_TEXTURE_BUFFER, texture);
    };
    // Every unit is bound whatever the flags, like the stale bindings a draw
    // can inherit; fSkin, fPose and warpType alone must gate the reads.
    bind(kBoneMatrixTextureUnit, buffers != nullptr ? buffers->boneTexture : 0);
    bind(kPoseDposTextureUnit, buffers != nullptr ? GetBufferTexture(buffers->poseDpos) : 0);
    bind(kPoseDnormalTextureUnit, buffers != nullptr ? GetBufferTexture(buffers->poseDnormal) : 0);
    bind(kPoseWeightTextureUnit, buffers != nullptr ? GetBufferTexture(buffers->poseWeights) : 0);
    bind(kWarpStateTextureUnit, buffers != nullptr ? GetBufferTexture(buffers->warpState) : 0);
    glActiveTexture(GL_TEXTURE0);

    GLuint vao = 0;
    GLuint vbo = 0;
    GLuint capture = 0;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(scene.vertices.size() * sizeof(GlobVertex)),
        scene.vertices.data(), GL_STATIC_DRAW);
    const auto attribute = [](GLuint index, GLint size, size_t offset)
    {
        glVertexAttribPointer(index, size, GL_FLOAT, GL_FALSE, sizeof(GlobVertex), reinterpret_cast<void*>(offset));
        glEnableVertexAttribArray(index);
    };
    attribute(0, 3, offsetof(GlobVertex, pos));
    attribute(1, 3, offsetof(GlobVertex, normal));
    attribute(2, 4, offsetof(GlobVertex, color));
    attribute(3, 2, offsetof(GlobVertex, uv));
    glVertexAttribIPointer(4, 4, GL_UNSIGNED_INT, sizeof(GlobVertex),
        reinterpret_cast<void*>(offsetof(GlobVertex, boneIndices)));
    glEnableVertexAttribArray(4);
    attribute(5, 4, offsetof(GlobVertex, boneWeights));

    const GLsizeiptr captureSize = static_cast<GLsizeiptr>(scene.vertices.size() * sizeof(GlobCapture));
    glGenBuffers(1, &capture);
    glBindBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, capture);
    glBufferData(GL_TRANSFORM_FEEDBACK_BUFFER, captureSize, nullptr, GL_STATIC_READ);
    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, capture);

    glEnable(GL_RASTERIZER_DISCARD);
    glBeginTransformFeedback(GL_POINTS);
    glDrawArrays(GL_POINTS, 0, static_cast<GLsizei>(scene.vertices.size()));
    glEndTransformFeedback();
    glDisable(GL_RASTERIZER_DISCARD);

    GlobCapture unwritten;
    unwritten.fill(-999.0f);
    std::vector<GlobCapture> captured(scene.vertices.size(), unwritten);
    glBindBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, capture);
    glGetBufferSubData(GL_TRANSFORM_FEEDBACK_BUFFER, 0, captureSize, captured.data());

    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, 0);
    glBindVertexArray(0);
    glDeleteBuffers(1, &capture);
    glDeleteBuffers(1, &vbo);
    glDeleteVertexArrays(1, &vao);
    for (GLint unit : { kBoneMatrixTextureUnit, kPoseDposTextureUnit, kPoseDnormalTextureUnit,
        kPoseWeightTextureUnit, kWarpStateTextureUnit })
        bind(unit, 0);
    glActiveTexture(GL_TEXTURE0);
    return captured;
}

// Runs the whole, unmodified glob.vert through transform feedback: skinning, poses and every warp type, against
// the per-object data uploaded the way glob.cpp and wr.cpp upload it.
void TestGlobVertexShader()
{
    std::printf("glob.vert (whole vertex shader, transform feedback vs glm):\n");
    std::vector<std::string> stubbed;
    std::string error;
    std::string source;
    try
    {
        source = StubBufferBlocks(CaneEmbeddedShader("glob.vert"), stubbed, error);
    }
    catch (const std::exception& exception)
    {
        error = exception.what();
    }
    std::string names;
    for (const std::string& name : stubbed)
        names += " " + name;
    std::printf("  stubbed %zu buffer blocks:%s\n", stubbed.size(), names.c_str());
    CHECK(stubbed.empty());
    if (!error.empty())
    {
        std::fprintf(stderr, "FAIL: glob.vert buffer blocks could not be stubbed: %s\n", error.c_str());
        CHECK(error.empty());
        return;
    }

    const GLuint program = LinkStages(
        CompileSource(GL_VERTEX_SHADER, "glob.vert", RewriteShaderVersion(source, kShaderVersionGL41)),
        CompileStage(GL_FRAGMENT_SHADER, "glob.frag"), error, CaptureGlobVertex);
    if (program == 0)
    {
        std::fprintf(stderr, "FAIL: glob vertex program did not build: %s\n", error.c_str());
        CHECK(program != 0);
        return;
    }
    // Five shadow and light samplers, the five per-object ones and the cache.
    CHECK(CheckSamplerUnits("glob vertex", program) == 11);

    // The RO block's std140 offsets must match ROGL's.
    struct { const char* name; size_t offset; } kRoMembers[] =
    {
        { "RO.model", offsetof(RoGl, model) },
        { "RO.warpType", offsetof(RoGl, warpType) },
        { "RO.warpCmat", offsetof(RoGl, warpCmat) },
        { "RO.warpCvtx", offsetof(RoGl, warpCvtx) },
        { "RO.amatDpos[0]", offsetof(RoGl, amatDpos) },
        { "RO.amatDuv[0]", offsetof(RoGl, amatDuv) },
        { "RO.trlk", offsetof(RoGl, trlk) },
    };
    for (const auto& member : kRoMembers)
    {
        GLuint index = GL_INVALID_INDEX;
        glGetUniformIndices(program, 1, &member.name, &index);
        GLint offset = -1;
        if (index != GL_INVALID_INDEX)
            glGetActiveUniformsiv(program, 1, &index, GL_UNIFORM_OFFSET, &offset);
        if (offset != static_cast<GLint>(member.offset))
            std::fprintf(stderr, "  %s at offset %d, ROGL has %zu\n", member.name, offset, member.offset);
        CHECK(offset == static_cast<GLint>(member.offset));
    }

    GlobScene scene;
    // Bone indices: v3 has some out of range, v4 has only indices in 4..15
    // (past the 4 bones but inside 4 * 4 texels), v5 has no weights.
    const auto vertex = [](glm::vec3 pos, glm::vec3 normal, glm::vec2 uv, glm::uvec4 bones, glm::vec4 weights)
    {
        return GlobVertex{ pos, normal, glm::vec4(0.5f, 0.6f, 0.7f, 1.0f), uv, bones, weights };
    };
    scene.vertices =
    {
        vertex({ -1.0f, 0.0f, 0.5f }, { 0.0f, 1.0f, 0.0f }, { 0.1f, 0.2f }, { 0, 1, 2, 3 }, { 0.4f, 0.3f, 0.2f, 0.1f }),
        vertex({ 1.0f, 0.2f, 0.1f }, { 0.7f, 0.7f, 0.0f }, { 0.9f, 0.3f }, { 2, 3, 0, 1 }, { 0.5f, 0.25f, 0.25f, 0.0f }),
        vertex({ 0.3f, 1.0f, -0.6f }, { 0.0f, 0.0f, 1.0f }, { 0.4f, 0.8f }, { 1, 0, 0, 0 }, { 1.0f, 0.0f, 0.0f, 0.0f }),
        vertex({ -0.2f, 0.9f, 0.7f }, { 1.0f, 0.0f, 0.0f }, { 0.6f, 0.1f }, { 3, 9, 0xFFFFFFFFu, 2 }, { 0.3f, 0.3f, 0.2f, 0.2f }),
        vertex({ 0.5f, -0.7f, 0.2f }, { 0.0f, -1.0f, 0.2f }, { 0.2f, 0.7f }, { 4, 5, 6, 7 }, { 0.25f, 0.25f, 0.25f, 0.25f }),
        vertex({ -0.6f, -0.4f, -0.3f }, { 0.3f, 0.2f, -1.0f }, { 0.5f, 0.5f }, { 0, 0, 0, 0 }, { 0.0f, 0.0f, 0.0f, 0.0f }),
    };
    const int vertexCount = static_cast<int>(scene.vertices.size());
    scene.model = glm::translate(glm::mat4(1.0f), glm::vec3(0.5f, -0.25f, 0.3f)) *
        glm::rotate(glm::mat4(1.0f), 0.2f, glm::normalize(glm::vec3(1.0f, 1.0f, 0.0f))) *
        glm::scale(glm::mat4(1.0f), glm::vec3(1.1f, 0.9f, 1.2f));
    scene.worldToClip = glm::perspective(glm::radians(60.0f), 4.0f / 3.0f, 0.1f, 100.0f) *
        glm::lookAt(glm::vec3(1.0f, 2.0f, 9.0f), glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    scene.poseCount = 2;
    scene.warpCmat = 2;
    scene.warpCvtx = vertexCount;
    for (int i = 0; i < 4; ++i)
    {
        const float f = static_cast<float>(i + 1);
        scene.amatDpos[i] = glm::rotate(glm::mat4(1.0f), 0.3f * f, glm::normalize(glm::vec3(1.0f, f, -1.0f))) *
            glm::scale(glm::mat4(1.0f), glm::vec3(0.2f * f, 0.15f, 0.1f * f));
        scene.amatDuv[i] = glm::scale(glm::mat4(1.0f), glm::vec3(0.05f * f, -0.04f * f, 0.3f)) *
            glm::rotate(glm::mat4(1.0f), 0.5f * f, glm::vec3(0.0f, 0.0f, 1.0f));
    }
    scene.uvOffsets = glm::vec2(0.125f, -0.0625f);

    GlobData full;
    full.bones =
    {
        glm::translate(glm::mat4(1.0f), glm::vec3(0.4f, -0.3f, 0.2f)) *
            glm::rotate(glm::mat4(1.0f), 0.3f, glm::vec3(0.0f, 0.0f, 1.0f)),
        glm::translate(glm::mat4(1.0f), glm::vec3(-0.5f, 0.25f, -0.4f)) *
            glm::rotate(glm::mat4(1.0f), -0.5f, glm::vec3(1.0f, 0.0f, 0.0f)) *
            glm::scale(glm::mat4(1.0f), glm::vec3(1.2f, 0.9f, 1.1f)),
        glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.6f, 0.3f)) *
            glm::rotate(glm::mat4(1.0f), 0.8f, glm::normalize(glm::vec3(1.0f, 2.0f, -1.0f))),
        glm::rotate(glm::mat4(1.0f), 0.4f, glm::vec3(0.0f, 1.0f, 0.0f)) *
            glm::scale(glm::mat4(1.0f), glm::vec3(0.8f, 1.25f, 0.9f)),
    };
    // A third globset pose this glob lacks, as in the celborder test.
    full.poseWeights = { 0.75f, -0.5f, 0.3f };
    for (int v = 0; v < vertexCount; ++v)
    {
        const float f = static_cast<float>(v);
        full.poseDpos.push_back(glm::vec4(0.05f * f - 0.3f, 0.2f - 0.03f * f, 0.1f, 0.0f));
        full.poseDpos.push_back(glm::vec4(-0.1f, 0.04f * f, 0.15f - 0.02f * f, 0.0f));
        full.poseDnormal.push_back(glm::vec4(0.2f - 0.1f * f, 0.05f * f, -0.15f, 0.0f));
        full.poseDnormal.push_back(glm::vec4(0.1f * f, -0.2f, 0.08f * f - 0.1f, 0.0f));
    }
    for (int imat = 0; imat < scene.warpCmat; ++imat)
    {
        for (int v = 0; v < vertexCount; ++v)
        {
            const float f = static_cast<float>(v);
            const float m = static_cast<float>(imat);
            full.warpState.push_back(glm::vec4(0.1f * f - 0.2f + 0.3f * m, 0.05f * f * (m + 1.0f) + 0.1f,
                0.2f * m - 0.07f * f, 1.0f - 0.1f * m));
        }
    }

    // Short buffers: the pose deltas stop part-way through v4 (dnormal one
    // delta earlier than dpos, so each length guard is hit on its own), the
    // warp state misses imat 1's last two vertices, and only one pose weight
    // is uploaded for a poseCount of 2.
    GlobData shortData = full;
    shortData.poseDpos.resize(static_cast<size_t>((vertexCount - 2) * scene.poseCount + 1));
    shortData.poseDnormal.resize(static_cast<size_t>((vertexCount - 2) * scene.poseCount));
    shortData.warpState.resize(static_cast<size_t>(scene.warpCmat * scene.warpCvtx - 2));
    GlobData oneWeight = full;
    oneWeight.poseWeights.resize(1);

    const GlobCase kCases[] =
    {
        { "static", false, false, kWarpNone, false, &full },
        { "skinned", true, false, kWarpNone, false, &full },
        { "posed", false, true, kWarpNone, false, &full },
        { "skinned+posed", true, true, kWarpNone, false, &full },
        { "warp pos", false, false, kWarpPos, false, &full },
        { "warp uv", false, false, kWarpUv, false, &full },
        { "warp both", false, false, kWarpBoth, false, &full },
        { "skinned+posed+warp both+uv anim", true, true, kWarpBoth, true, &full },
        { "posed, short deltas", false, true, kWarpNone, false, &shortData },
        { "warp both, short state", false, false, kWarpBoth, false, &shortData },
        { "posed, one weight", false, true, kWarpNone, false, &oneWeight },
    };

    glUseProgram(program);
    // glob.frag's material and shadow-array samplers, set as gl.cpp sets them;
    // left at unit 0 together, the draw fails with GL_INVALID_OPERATION.
    glUniform1i(glGetUniformLocation(program, "ambientMap"), 0);
    glUniform1i(glGetUniformLocation(program, "diffuseMap"), 1);
    glUniform1i(glGetUniformLocation(program, "saturateMap"), 2);
    glUniform1i(glGetUniformLocation(program, "shadowTextureArray"), 3);
    // No shadow or light textures: their counts read zero.
    for (GLint unit : { kShadowTextureUnit, kActiveShadowTextureUnit, kLightTextureUnit, kActiveLightTextureUnit,
        kTvLightTextureUnit })
    {
        glActiveTexture(GL_TEXTURE0 + unit);
        glBindTexture(GL_TEXTURE_BUFFER, 0);
    }
    glActiveTexture(GL_TEXTURE0);
    struct { glm::mat4 matWorldToClip; glm::vec4 cameraPos; } cmgl{ scene.worldToClip, glm::vec4(1.0f, 2.0f, 9.0f, 1.0f) };
    const GLuint cmglBuffer = CreateUniformBuffer(program, "CMGL", FindBlockBinding("CMGL"), &cmgl, sizeof(cmgl));
    const RoGl roZero{};
    const GLuint roBuffer = CreateUniformBuffer(program, "RO", FindBlockBinding("RO"), &roZero, sizeof(roZero));

    const GlobBuffers fullBuffers = UploadGlobData(full);
    const GlobBuffers shortBuffers = UploadGlobData(shortData);
    const GlobBuffers oneWeightBuffers = UploadGlobData(oneWeight);
    const auto buffersFor = [&](const GlobData* data)
    {
        return data == &full ? &fullBuffers : data == &shortData ? &shortBuffers : &oneWeightBuffers;
    };

    const auto report = [&](const std::string& name, const std::vector<GlobCapture>& captured, const GlobCase& c)
    {
        int bad = 0;
        for (int v = 0; v < vertexCount; ++v)
        {
            const GlobCapture& got = captured[static_cast<size_t>(v)];
            const GlobCapture want = ReferenceGlobVertex(scene, c, v);
            if (NearlyEqualCapture(got, want))
                continue;
            ++bad;
            std::printf("  MISMATCH %s v%d: got pos (%.5f, %.5f, %.5f, %.5f) n (%.5f, %.5f, %.5f) uv (%.5f, %.5f)\n"
                "      want pos (%.5f, %.5f, %.5f, %.5f) n (%.5f, %.5f, %.5f) uv (%.5f, %.5f)\n",
                name.c_str(), v, got[0], got[1], got[2], got[3], got[4], got[5], got[6], got[7], got[8],
                want[0], want[1], want[2], want[3], want[4], want[5], want[6], want[7], want[8]);
        }
        if (bad == 0)
            std::printf("  ok %s: %d vertices (gl_Position, worldNormal, texcoord)\n", name.c_str(), vertexCount);
        CHECK(bad == 0);
    };

    for (const GlobCase& c : kCases)
        report(c.name, RunGlob(program, roBuffer, scene, c, buffersFor(c.data)), c);

    // fSkin, fPose and a warp type with nothing bound: textureSize() is zero,
    // so every guard skips and the vertex comes out static.
    const GlobCase unboundCase{ "skinned+posed+warp both, nothing bound", true, true, kWarpBoth, false, &full };
    const GlobCase staticCase{ "static", false, false, kWarpNone, false, &full };
    {
        const std::vector<GlobCapture> captured = RunGlob(program, roBuffer, scene, unboundCase, nullptr);
        int bad = 0;
        for (int v = 0; v < vertexCount; ++v)
            bad += NearlyEqualCapture(captured[static_cast<size_t>(v)], ReferenceGlobVertex(scene, staticCase, v)) ? 0 : 1;
        std::printf("  %s %s: %d/%d vertices match the static reference\n", bad == 0 ? "ok" : "MISMATCH",
            unboundCase.name, vertexCount - bad, vertexCount);
        CHECK(bad == 0);
    }

    // The cases must exercise what they claim, or they prove nothing.
    const auto ref = [&](const GlobCase& c, int v) { return ReferenceGlobVertex(scene, c, v); };
    const auto differs = [&](const GlobCase& a, const GlobCase& b, int v) { return !NearlyEqualCapture(ref(a, v), ref(b, v)); };
    const GlobCase& kStatic = kCases[0];
    const GlobCase& kSkinned = kCases[1];
    const GlobCase& kPosed = kCases[2];
    const GlobCase& kWarpPosCase = kCases[4];
    const GlobCase& kWarpUvCase = kCases[5];
    const GlobCase& kShortPose = kCases[8];
    const GlobCase& kShortWarp = kCases[9];
    const GlobCase& kOneWeight = kCases[10];
    for (int v : { 0, 1, 2, 3 })
        CHECK(differs(kSkinned, kStatic, v));
    // v4's bones are all past the last bone and v5 has no weights.
    CHECK(!differs(kSkinned, kStatic, 4));
    CHECK(!differs(kSkinned, kStatic, 5));
    for (int v = 0; v < vertexCount; ++v)
    {
        CHECK(differs(kPosed, kStatic, v));
        CHECK(differs(kWarpPosCase, kStatic, v));
        CHECK(differs(kWarpUvCase, kStatic, v));
        CHECK(differs(kCases[6], kWarpPosCase, v));
        CHECK(differs(kCases[6], kWarpUvCase, v));
        CHECK(differs(kOneWeight, kPosed, v));
    }
    // Each vertex's deltas differ, so poseBase must be per vertex.
    CHECK(ref(kPosed, 0)[0] - ref(kStatic, 0)[0] != ref(kPosed, 1)[0] - ref(kStatic, 1)[0]);
    // Short deltas: v0-v3 whole, v4 loses pose 0 to the dnormal guard and
    // pose 1 to the dpos guard, v5 loses both.
    for (int v : { 0, 1, 2, 3 })
        CHECK(!differs(kShortPose, kPosed, v));
    CHECK(differs(kShortPose, kPosed, 4));
    CHECK(!differs(kShortPose, kStatic, 4));
    CHECK(!differs(kShortPose, kStatic, 5));
    // Short warp state: only v4 and v5 lose imat 1.
    for (int v : { 0, 1, 2, 3 })
        CHECK(!differs(kShortWarp, kCases[6], v));
    CHECK(differs(kShortWarp, kCases[6], 4));
    CHECK(differs(kShortWarp, kStatic, 4));

    const GLenum glError = glGetError();
    if (glError != GL_NO_ERROR)
        std::fprintf(stderr, "glob vertex shader raised GL error 0x%04x\n", glError);
    CHECK(glError == GL_NO_ERROR);

    glUseProgram(0);
    glBindBufferBase(GL_UNIFORM_BUFFER, FindBlockBinding("CMGL"), 0);
    glBindBufferBase(GL_UNIFORM_BUFFER, FindBlockBinding("RO"), 0);
    glDeleteBuffers(1, &cmglBuffer);
    glDeleteBuffers(1, &roBuffer);
    DeleteGlobBuffers(fullBuffers);
    DeleteGlobBuffers(shortBuffers);
    DeleteGlobBuffers(oneWeightBuffers);
    CHECK(BufferTextures().empty());
    glDeleteProgram(program);
}

// ---- glob material cache bake ------------------------------------------------

struct BakeDraw
{
    GLint baseVertex;
    GLsizei count;
    bool skin;
    float unSelfIllum;
    bool subglob; // false: a vertex no subglob covers, drawn but never baked
};

// One frame of DrawGlob's subglob loop with the glob program current: per
// draw, fSkin and unSelfIllum, then (when `bake`) BakeCachedMaterial into
// `cache` as DrawGlob does, then the draw itself as points, capturing
// `material` through the same varyings into the same records of a fresh
// buffer. `primitives` gets the points each bake wrote.
std::vector<CachedMaterial> RunCacheFrame(GLuint program, GLuint vao, GLuint cache, int vertexCount,
    const std::vector<BakeDraw>& draws, bool bake, std::vector<GLuint>* primitives = nullptr)
{
    CachedMaterial sentinel{};
    sentinel.ambient = -7.0f;
    std::fill(std::begin(sentinel.pad), std::end(sentinel.pad), -7.0f);
    sentinel.midtone = glm::vec4(-7.0f);
    sentinel.light = glm::vec4(-7.0f);
    std::vector<CachedMaterial> captured(static_cast<size_t>(vertexCount), sentinel);

    GLuint capture = 0;
    glGenBuffers(1, &capture);
    glBindBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, capture);
    glBufferData(GL_TRANSFORM_FEEDBACK_BUFFER, static_cast<GLsizeiptr>(captured.size() * sizeof(CachedMaterial)),
        captured.data(), GL_STATIC_READ);
    glBindBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, 0);

    GLuint query = 0;
    glGenQueries(1, &query);
    glBindVertexArray(vao);
    const GLint bakeLocation = glGetUniformLocation(program, "fCacheBake");
    for (const BakeDraw& draw : draws)
    {
        glUniform1i(glGetUniformLocation(program, "fSkin"), draw.skin ? 1 : 0);
        glUniform1f(glGetUniformLocation(program, "unSelfIllum"), draw.unSelfIllum);

        if (bake && draw.subglob)
        {
            glBeginQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN, query);
            BakeCachedMaterial(cache, draw.baseVertex, draw.count, bakeLocation);
            glEndQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN);
            GLuint written = 0;
            glGetQueryObjectuiv(query, GL_QUERY_RESULT, &written);
            if (primitives != nullptr)
                primitives->push_back(written);
        }

        glBindBufferRange(GL_TRANSFORM_FEEDBACK_BUFFER, 0, capture,
            static_cast<GLintptr>(draw.baseVertex) * kCachedMaterialStride,
            static_cast<GLsizeiptr>(draw.count) * kCachedMaterialStride);
        glEnable(GL_RASTERIZER_DISCARD);
        glBeginTransformFeedback(GL_POINTS);
        glDrawArrays(GL_POINTS, draw.baseVertex, draw.count);
        glEndTransformFeedback();
        glDisable(GL_RASTERIZER_DISCARD);
        glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, 0);
    }
    glBindVertexArray(0);
    glDeleteQueries(1, &query);

    glBindBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, capture);
    glGetBufferSubData(GL_TRANSFORM_FEEDBACK_BUFFER, 0,
        static_cast<GLsizeiptr>(captured.size() * sizeof(CachedMaterial)), captured.data());
    glBindBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, 0);
    glDeleteBuffers(1, &capture);
    return captured;
}

bool NearlyEqualMaterial(const CachedMaterial& actual, const CachedMaterial& expected)
{
    const float a[9] = { actual.ambient, actual.midtone.x, actual.midtone.y, actual.midtone.z, actual.midtone.w,
        actual.light.x, actual.light.y, actual.light.z, actual.light.w };
    const float e[9] = { expected.ambient, expected.midtone.x, expected.midtone.y, expected.midtone.z,
        expected.midtone.w, expected.light.x, expected.light.y, expected.light.z, expected.light.w };
    for (int i = 0; i < 9; ++i)
    {
        if (!(std::fabs(a[i] - e[i]) <= 1e-6f * std::fmax(1.0f, std::fabs(e[i]))))
            return false;
    }
    return true;
}

// Counts the vertices in [first, last) whose materials differ, printing each.
int CompareMaterials(const char* label, const std::vector<CachedMaterial>& actual,
    const std::vector<CachedMaterial>& expected, int first, int last)
{
    int bad = 0;
    for (int v = first; v < last; ++v)
    {
        const CachedMaterial& got = actual[static_cast<size_t>(v)];
        const CachedMaterial& want = expected[static_cast<size_t>(v)];
        if (NearlyEqualMaterial(got, want))
            continue;
        ++bad;
        std::printf("  MISMATCH %s v%d: got a %.7f m (%.7f %.7f %.7f %.7f) l (%.7f %.7f %.7f %.7f)\n"
            "      want a %.7f m (%.7f %.7f %.7f %.7f) l (%.7f %.7f %.7f %.7f)\n", label, v,
            got.ambient, got.midtone.x, got.midtone.y, got.midtone.z, got.midtone.w,
            got.light.x, got.light.y, got.light.z, got.light.w,
            want.ambient, want.midtone.x, want.midtone.y, want.midtone.z, want.midtone.w,
            want.light.x, want.light.y, want.light.z, want.light.w);
    }
    return bad;
}

// Bakes glob.vert's static three-way lighting into a cache with
// BakeCachedMaterial, per subglob as DrawGlob does, then reads it back with a
// TRLK_Baked draw. Three subglobs of different sizes (two at a non-zero
// baseVertex), skinned and not, with a static direction light, a static
// position light and a dynamic light; one vertex past every subglob.
void TestGlobCacheBake()
{
    std::printf("glob material cache (transform feedback bake, buffer texture read):\n");
    std::string error;
    const GLuint program = LinkStages(CompileStage(GL_VERTEX_SHADER, "glob.vert"),
        CompileStage(GL_FRAGMENT_SHADER, "glob.frag"), error, CaptureGlobMaterial);
    if (program == 0)
    {
        std::fprintf(stderr, "FAIL: glob program with cache varyings did not build: %s\n", error.c_str());
        CHECK(program != 0);
        return;
    }
    CHECK(glGetUniformLocation(program, "fCacheBake") >= 0);
    CHECK(glGetUniformLocation(program, "cachedMaterialTex") >= 0);
    CHECK(UniformType(program, "cachedMaterialTex") == GL_SAMPLER_BUFFER);

    // Subglobs packed from 0 as glob.cpp packs them; vertex 12 is past all of
    // them (and past the cache), so a Baked read must take the guard.
    const std::vector<BakeDraw> draws =
    {
        { 0, 3, true, 2000.0f, true },
        { 3, 5, false, 6000.0f, true },
        { 8, 4, true, 0.0f, true },
        { 12, 1, false, 4000.0f, false },
    };
    const int cachedVertices = 12;
    const int vertexCount = 13;

    std::vector<GlobVertex> vertices;
    for (int v = 0; v < vertexCount; ++v)
    {
        const float f = static_cast<float>(v);
        GlobVertex vertex{};
        vertex.pos = glm::vec3(std::sin(f) * 0.8f, 0.3f * f - 1.5f, std::cos(1.7f * f) * 0.6f);
        vertex.normal = glm::normalize(glm::vec3(std::cos(0.9f * f), 0.5f + 0.1f * f, std::sin(1.3f * f)));
        vertex.color = glm::vec4(0.4f + 0.04f * f, 0.7f - 0.03f * f, 0.3f + 0.02f * f, 1.0f);
        vertex.uv = glm::vec2(0.1f * f, 0.05f * f);
        vertex.boneIndices = glm::uvec4(v % 2, (v + 1) % 2, 0, 0);
        vertex.boneWeights = glm::vec4(0.7f, 0.3f, 0.0f, 0.0f);
        vertices.push_back(vertex);
    }

    GLuint vao = 0;
    GLuint vbo = 0;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(vertices.size() * sizeof(GlobVertex)), vertices.data(),
        GL_STATIC_DRAW);
    const auto attribute = [](GLuint index, GLint size, size_t offset)
    {
        glVertexAttribPointer(index, size, GL_FLOAT, GL_FALSE, sizeof(GlobVertex), reinterpret_cast<void*>(offset));
        glEnableVertexAttribArray(index);
    };
    attribute(0, 3, offsetof(GlobVertex, pos));
    attribute(1, 3, offsetof(GlobVertex, normal));
    attribute(2, 4, offsetof(GlobVertex, color));
    attribute(3, 2, offsetof(GlobVertex, uv));
    glVertexAttribIPointer(4, 4, GL_UNSIGNED_INT, sizeof(GlobVertex),
        reinterpret_cast<void*>(offsetof(GlobVertex, boneIndices)));
    glEnableVertexAttribArray(4);
    attribute(5, 4, offsetof(GlobVertex, boneWeights));
    glBindVertexArray(0);

    // Lights as light.cpp uploads them: a header, then the records. The
    // relight pass walks the whole table and skips fDynamic lights; the
    // dynamic pass takes light 2 from the active list.
    std::vector<LightBlk> lights(3);
    lights[0].lightk = 0;
    lights[0].fActive = 1;
    lights[0].dir = glm::vec4(glm::normalize(glm::vec3(0.3f, -0.8f, 0.5f)), 0.0f);
    lights[0].color = glm::vec4(0.35f, 0.3f, 0.25f, 1.0f);
    lights[0].ru = glm::vec4(0.3f, 0.25f, 0.2f, 0.0f);
    lights[0].du = glm::vec4(0.1f, 0.15f, 0.05f, 0.0f);
    lights[1].lightk = 1;
    lights[1].fActive = 1;
    lights[1].pos = glm::vec4(1.5f, 1.0f, 2.0f, 1.0f);
    lights[1].color = glm::vec4(0.2f, 0.3f, 0.15f, 1.0f);
    lights[1].constant = 0.2f;
    lights[1].invDst = 0.5f;
    lights[1].dst = 10.0f;
    lights[1].ru = glm::vec4(0.2f, 0.2f, 0.3f, 0.0f);
    lights[1].du = glm::vec4(0.05f, 0.1f, 0.1f, 0.0f);
    lights[2].lightk = 0;
    lights[2].fActive = 1;
    lights[2].fDynamic = 1;
    lights[2].dir = glm::vec4(glm::normalize(glm::vec3(-0.5f, 0.2f, 0.8f)), 0.0f);
    lights[2].color = glm::vec4(0.15f, 0.2f, 0.35f, 1.0f);
    lights[2].ru = glm::vec4(0.2f, 0.3f, 0.25f, 0.0f);
    lights[2].du = glm::vec4(0.05f, 0.05f, 0.1f, 0.0f);
    const GLuint lightBuffer = CreateLightBuffer(true, 3, lights, 0, kLightTextureFormat);
    ActiveLights active = {};
    active.numStaticLights = 2;
    active.staticLightIndices[0] = 0;
    active.staticLightIndices[1] = 1;
    active.numDynamicLights = 1;
    active.dynamicLightIndices[0] = 2;
    const GLuint activeBuffer = CreateActiveLightBuffer(active);

    GlobData data;
    data.bones =
    {
        glm::rotate(glm::mat4(1.0f), 0.9f, glm::normalize(glm::vec3(1.0f, 0.5f, 0.0f))),
        glm::translate(glm::mat4(1.0f), glm::vec3(0.2f, -0.1f, 0.3f)) *
            glm::rotate(glm::mat4(1.0f), -1.1f, glm::vec3(0.0f, 0.0f, 1.0f)),
    };
    // Unread here (no poses, no warp); one element each keeps the stores non-empty.
    data.poseDpos = { glm::vec4(0.0f) };
    data.poseDnormal = { glm::vec4(0.0f) };
    data.poseWeights = { 0.0f };
    data.warpState = { glm::vec4(0.0f) };
    const GlobBuffers globBuffers = UploadGlobData(data);

    // The cache as glob.cpp creates it (sized from the subglobs), filled with
    // a sentinel here so unbaked records and the skipped padding show.
    std::vector<CachedMaterial> initial(static_cast<size_t>(cachedVertices));
    for (CachedMaterial& record : initial)
    {
        record.ambient = -7.0f;
        std::fill(std::begin(record.pad), std::end(record.pad), -7.0f);
        record.midtone = glm::vec4(-7.0f);
        record.light = glm::vec4(-7.0f);
    }
    GLuint cache = 0;
    glGenBuffers(1, &cache);
    glBindBuffer(GL_TEXTURE_BUFFER, cache);
    glBufferData(GL_TEXTURE_BUFFER, static_cast<GLsizeiptr>(initial.size() * sizeof(CachedMaterial)),
        initial.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_TEXTURE_BUFFER, 0);
    CHECK(CreateBufferTexture(cache, kCachedMaterialTextureFormat) != 0);

    glUseProgram(program);
    glUniform1i(glGetUniformLocation(program, "ambientMap"), 0);
    glUniform1i(glGetUniformLocation(program, "diffuseMap"), 1);
    glUniform1i(glGetUniformLocation(program, "saturateMap"), 2);
    glUniform1i(glGetUniformLocation(program, "shadowTextureArray"), 3);
    glUniform1f(glGetUniformLocation(program, "swp.uShadow"), 0.25f);
    glUniform1f(glGetUniformLocation(program, "swp.uMidtone"), 0.2f);
    glUniform1i(glGetUniformLocation(program, "swp.fogType"), 0);
    glUniform1i(glGetUniformLocation(program, "rko"), 1);
    glUniform1i(glGetUniformLocation(program, "fPose"), 0);
    glUniform1i(glGetUniformLocation(program, "fAnimateUv"), 0);
    const auto bind = [](GLint unit, GLuint texture)
    {
        glActiveTexture(GL_TEXTURE0 + unit);
        glBindTexture(GL_TEXTURE_BUFFER, texture);
    };
    bind(kLightTextureUnit, GetBufferTexture(lightBuffer));
    bind(kActiveLightTextureUnit, GetBufferTexture(activeBuffer));
    bind(kBoneMatrixTextureUnit, globBuffers.boneTexture);
    for (GLint unit : { kShadowTextureUnit, kActiveShadowTextureUnit, kTvLightTextureUnit, kWarpStateTextureUnit,
        kPoseDposTextureUnit, kPoseDnormalTextureUnit, kPoseWeightTextureUnit, kCachedMaterialTextureUnit })
        bind(unit, 0);
    glActiveTexture(GL_TEXTURE0);

    const glm::mat4 worldToClip = glm::perspective(glm::radians(60.0f), 4.0f / 3.0f, 0.1f, 100.0f) *
        glm::lookAt(glm::vec3(1.0f, 2.0f, 9.0f), glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    struct { glm::mat4 matWorldToClip; glm::vec4 cameraPos; } cmgl{ worldToClip, glm::vec4(1.0f, 2.0f, 9.0f, 1.0f) };
    const GLuint cmglBuffer = CreateUniformBuffer(program, "CMGL", FindBlockBinding("CMGL"), &cmgl, sizeof(cmgl));
    RoGl ro{};
    ro.model = glm::translate(glm::mat4(1.0f), glm::vec3(0.2f, 0.1f, -0.3f)) *
        glm::rotate(glm::mat4(1.0f), 0.4f, glm::normalize(glm::vec3(0.0f, 1.0f, 0.3f)));
    ro.uAlpha = 1.0f;
    ro.sRadius = 2.0f;
    ro.posCenter = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
    const GLuint roBuffer = CreateUniformBuffer(program, "RO", FindBlockBinding("RO"), &ro, sizeof(ro));
    const auto setRo = [&](int trlk, int fDynamicLight)
    {
        ro.trlk = trlk;
        ro.fDynamicLight = fDynamicLight;
        glBindBuffer(GL_UNIFORM_BUFFER, roBuffer);
        glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(ro), &ro);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
    };
    constexpr int kRelight = 0;
    constexpr int kBaked = 1;

    // (A) the relight frame: bake each subglob, then draw it lit live with
    // the dynamic light. (A') the same frame without the bake. (S) the frame
    // with the dynamic light off: what the cache must hold.
    setRo(kRelight, 1);
    std::vector<GLuint> primitives;
    const std::vector<CachedMaterial> relight = RunCacheFrame(program, vao, cache, vertexCount, draws, true, &primitives);
    std::vector<CachedMaterial> cached(static_cast<size_t>(cachedVertices));
    glBindBuffer(GL_TEXTURE_BUFFER, cache);
    glGetBufferSubData(GL_TEXTURE_BUFFER, 0, static_cast<GLsizeiptr>(cached.size() * sizeof(CachedMaterial)),
        cached.data());
    glBindBuffer(GL_TEXTURE_BUFFER, 0);
    GLint feedbackBinding = -1;
    glGetIntegerv(GL_TRANSFORM_FEEDBACK_BUFFER_BINDING, &feedbackBinding);
    const GLboolean discard = glIsEnabled(GL_RASTERIZER_DISCARD);
    GLint bakeFlag = -1;
    glGetUniformiv(program, glGetUniformLocation(program, "fCacheBake"), &bakeFlag);
    const std::vector<CachedMaterial> relightNoBake = RunCacheFrame(program, vao, cache, vertexCount, draws, false);
    setRo(kRelight, 0);
    const std::vector<CachedMaterial> staticOnly = RunCacheFrame(program, vao, cache, vertexCount, draws, false);
    std::vector<BakeDraw> unskinned = draws;
    for (BakeDraw& draw : unskinned)
        draw.skin = false;
    const std::vector<CachedMaterial> staticUnskinned = RunCacheFrame(program, vao, cache, vertexCount, unskinned, false);

    // (B) the baked frame reads the cache back, without and with the
    // dynamic light on top.
    bind(kCachedMaterialTextureUnit, GetBufferTexture(cache));
    glActiveTexture(GL_TEXTURE0);
    setRo(kBaked, 0);
    const std::vector<CachedMaterial> baked = RunCacheFrame(program, vao, cache, vertexCount, draws, false);
    setRo(kBaked, 1);
    const std::vector<CachedMaterial> bakedDynamic = RunCacheFrame(program, vao, cache, vertexCount, draws, false);

    // Each bake wrote exactly its subglob's points.
    bool primitivesOk = primitives.size() == 3;
    for (size_t i = 0; i < primitives.size() && i < 3; ++i)
        primitivesOk = primitivesOk && primitives[i] == static_cast<GLuint>(draws[i].count);
    std::printf("  bake points written per subglob: %u %u %u (want 3 5 4)\n",
        primitives.size() > 0 ? primitives[0] : 0u, primitives.size() > 1 ? primitives[1] : 0u,
        primitives.size() > 2 ? primitives[2] : 0u);
    CHECK(primitivesOk);
    CHECK(feedbackBinding == 0);
    CHECK(discard == GL_FALSE);
    CHECK(bakeFlag == 0);

    // The cache holds the static-only lighting, each record at its vertex,
    // with the skipped padding untouched.
    int bad = CompareMaterials("cache vs static-only", cached, staticOnly, 0, cachedVertices);
    int padTouched = 0;
    for (const CachedMaterial& record : cached)
        padTouched += record.pad[0] == -7.0f && record.pad[1] == -7.0f && record.pad[2] == -7.0f ? 0 : 1;
    std::printf("  %s cache: %d/%d records equal the static-only frame, %d paddings touched\n",
        bad == 0 && padTouched == 0 ? "ok" : "MISMATCH", cachedVertices - bad, cachedVertices, padTouched);
    CHECK(bad == 0);
    CHECK(padTouched == 0);

    // The baked read returns them; the vertex past the cache reads zero.
    bad = CompareMaterials("baked read vs static-only", baked, staticOnly, 0, cachedVertices);
    std::printf("  %s baked read: %d/%d vertices equal the static-only frame\n", bad == 0 ? "ok" : "MISMATCH",
        cachedVertices - bad, cachedVertices);
    CHECK(bad == 0);
    const CachedMaterial& guarded = baked[static_cast<size_t>(cachedVertices)];
    const bool guardZero = guarded.ambient == 0.0f && guarded.midtone == glm::vec4(0.0f) &&
        guarded.light == glm::vec4(0.0f);
    std::printf("  %s guard: v%d (past every subglob) reads a %.3f m.r %.3f l.r %.3f\n", guardZero ? "ok" : "MISMATCH",
        cachedVertices, guarded.ambient, guarded.midtone.x, guarded.light.x);
    CHECK(guardZero);

    // The bake does not change what the relight frame draws, and cached
    // static plus the dynamic light equals the live relight.
    bad = CompareMaterials("relight with vs without bake", relight, relightNoBake, 0, vertexCount);
    std::printf("  %s relight frame: %d/%d vertices unchanged by the bake\n", bad == 0 ? "ok" : "MISMATCH",
        vertexCount - bad, vertexCount);
    CHECK(bad == 0);
    bad = CompareMaterials("baked + dynamic vs relight", bakedDynamic, relight, 0, cachedVertices);
    std::printf("  %s baked + dynamic: %d/%d vertices equal the live relight\n", bad == 0 ? "ok" : "MISMATCH",
        cachedVertices - bad, cachedVertices);
    CHECK(bad == 0);

    // The scene must exercise what it claims: the dynamic light moves every
    // subglob's material, and skinning moves the skinned subglobs'.
    for (int i = 0; i < 3; ++i)
    {
        int dynamicMoves = 0;
        int skinMoves = 0;
        for (int v = draws[i].baseVertex; v < draws[i].baseVertex + draws[i].count; ++v)
        {
            dynamicMoves += NearlyEqualMaterial(relight[static_cast<size_t>(v)], staticOnly[static_cast<size_t>(v)]) ? 0 : 1;
            skinMoves += NearlyEqualMaterial(staticOnly[static_cast<size_t>(v)],
                staticUnskinned[static_cast<size_t>(v)]) ? 0 : 1;
        }
        std::printf("  subglob %d (base %d, %d vertices, fSkin %d): dynamic light moves %d, skinning moves %d\n", i,
            draws[i].baseVertex, draws[i].count, draws[i].skin ? 1 : 0, dynamicMoves, skinMoves);
        CHECK(dynamicMoves > 0);
        CHECK(draws[i].skin ? skinMoves > 0 : skinMoves == 0);
    }
    CHECK(staticOnly[0].ambient > 0.0f && staticOnly[0].light.x > 0.0f);

    const GLenum glError = glGetError();
    if (glError != GL_NO_ERROR)
        std::fprintf(stderr, "glob cache bake raised GL error 0x%04x\n", glError);
    CHECK(glError == GL_NO_ERROR);

    glUseProgram(0);
    for (GLint unit : { kLightTextureUnit, kActiveLightTextureUnit, kBoneMatrixTextureUnit, kCachedMaterialTextureUnit })
        bind(unit, 0);
    glActiveTexture(GL_TEXTURE0);
    glBindBufferBase(GL_UNIFORM_BUFFER, FindBlockBinding("CMGL"), 0);
    glBindBufferBase(GL_UNIFORM_BUFFER, FindBlockBinding("RO"), 0);
    glDeleteBuffers(1, &cmglBuffer);
    glDeleteBuffers(1, &roBuffer);
    DeleteBufferWithTexture(cache);
    DeleteBufferWithTexture(lightBuffer);
    DeleteBufferWithTexture(activeBuffer);
    DeleteGlobBuffers(globBuffers);
    CHECK(BufferTextures().empty());
    glDeleteBuffers(1, &vbo);
    glDeleteVertexArrays(1, &vao);
    glDeleteProgram(program);
}

void PrintExtensionProbe()
{
    static const char* const kExtensions[] =
    {
        "GL_ARB_texture_storage",
        "GL_ARB_shader_storage_buffer_object",
        "GL_ARB_shading_language_420pack",
        "GL_ARB_texture_buffer_object",
    };
    GLint count = 0;
    glGetIntegerv(GL_NUM_EXTENSIONS, &count);
    std::printf("info: %d extensions reported\n", count);
    for (const char* wanted : kExtensions)
    {
        bool present = false;
        for (GLint i = 0; i < count && !present; ++i)
        {
            const char* name = reinterpret_cast<const char*>(glGetStringi(GL_EXTENSIONS, static_cast<GLuint>(i)));
            present = name != nullptr && std::string(name) == wanted;
        }
        std::printf("info: %s %s\n", wanted, present ? "present" : "absent");
    }
}
}

int main()
{
    // No Dock icon, and never take focus from the user.
    cane_test::MakeBackgroundOnly();
    if (glfwInit() != GLFW_TRUE)
    {
        std::printf("SKIP: glfwInit failed (no display?)\n");
        return kSkipReturnCode;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    GLFWwindow* window = glfwCreateWindow(64, 64, "cane_shader_tests", nullptr, nullptr);
    if (window == nullptr)
    {
        std::printf("SKIP: could not create a 4.1 core window\n");
        glfwTerminate();
        return kSkipReturnCode;
    }
    // A test process must stay in the background (no Dock icon, no focus).
    if (cane_test::ActivationPolicy() != cane_test::kPolicyProhibited)
    {
        std::printf("FAIL: the test process is not background-only (activation policy %ld)\n",
            cane_test::ActivationPolicy());
        glfwTerminate();
        return 1;
    }
    glfwMakeContextCurrent(window);
    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)))
    {
        std::fprintf(stderr, "FAIL: glad could not load GL functions\n");
        glfwTerminate();
        return 1;
    }
    std::printf("info: GL_VERSION %s\n", reinterpret_cast<const char*>(glGetString(GL_VERSION)));
    std::printf("info: GL_RENDERER %s\n", reinterpret_cast<const char*>(glGetString(GL_RENDERER)));
    PrintExtensionProbe();
    TestTexStorageFallback();
    TestBufferSamplerPlaceholders();

    struct Pair
    {
        const char* label;
        const char* vertex;
        const char* fragment;
        bool required;
        int blocks;   // uniform blocks expected in a required pair
        int samplers; // kSamplerUnits entries expected in a required pair
    };
    static const Pair kPairs[] =
    {
        { "screen", "screen.vert", "screen.frag", true, 0, 0 },
        { "geom", "geom.vert", "geom.frag", true, 1, 0 },
        { "blot", "blot.vert", "blot.frag", true, 0, 0 },
        { "blip", "blip.vert", "blip.frag", true, 2, 0 },
        { "dysh", "dysh.vert", "dysh.frag", true, 0, 1 },
        { "glob", "glob.vert", "glob.frag", true, 2, 11 },
        // vert + frag only, as the 4.3 build links it; the game's GL 4.1
        // program adds celborder.geom (TestCelBorderOutlineWidth builds that).
        { "celborder", "celborder.vert", "celborder.frag", true, 2, 4 },
    };

    for (const Pair& pair : kPairs)
    {
        std::string error;
        const GLuint program = BuildProgram(pair.vertex, pair.fragment, error);
        if (program == 0)
        {
            if (pair.required)
            {
                std::fprintf(stderr, "FAIL: %s did not build: %s\n", pair.label, error.c_str());
                CHECK(program != 0);
            }
            else
            {
                std::printf("info: %s fails as expected: %s\n", pair.label, error.c_str());
            }
            continue;
        }
        std::printf("%s: %s compiled and linked\n", pair.required ? "ok" : "info", pair.label);
        const int blocks = CheckBlockBindings(pair.label, program);
        const int samplers = CheckSamplerUnits(pair.label, program);
        if (pair.required)
        {
            CHECK(blocks == pair.blocks);
            CHECK(samplers == pair.samplers);
        }
        glDeleteProgram(program);
    }

    TestDyshSkinning();
    TestCelBorderEdges();
    TestCelBorderOutlineWidth();
    TestBufferTextureSidecar();
    TestBlipInstanceRanges();
    GLint feedbackComponents = 0;
    glGetIntegerv(GL_MAX_TRANSFORM_FEEDBACK_INTERLEAVED_COMPONENTS, &feedbackComponents);
    std::printf("info: GL_MAX_TRANSFORM_FEEDBACK_INTERLEAVED_COMPONENTS %d (shadow probe needs %d)\n",
        feedbackComponents, kShadowProbeVaryingCount * 4);
    TestGlobShadowFetch("glob.vert", true);
    TestGlobShadowFetch("glob.frag", false);
    std::printf("info: light probe needs %d interleaved components\n", kLightProbeVaryingCount * 4);
    TestGlobLightFetch();
    TestGlobProgramLinks();
    TestGlobVertexShader();
    TestGlobCacheBake();

    glfwDestroyWindow(window);
    glfwTerminate();

    if (g_failures != 0)
    {
        std::fprintf(stderr, "%d of %d checks failed\n", g_failures, g_checks);
        return 1;
    }
    std::printf("All %d checks passed\n", g_checks);
    return 0;
}
