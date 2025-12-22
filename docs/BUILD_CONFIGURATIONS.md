# Build Configurations Guide

This guide explains how to configure the build system for different scenarios.

## Default Behavior

**When you run `cmake ..` with no options:**
- ✅ Mock backend: **ENABLED**
- ✅ Tests: **ENABLED** (Mock backend tests)
- ✅ Coverage: **ENABLED**
- ❌ Examples: **DISABLED** (no Mock example exists)

## Quick Reference

### Build Only FFmpeg Backend

```bash
cmake .. -DENABLE_FFMPEG_BACKEND=ON
```

**Result:**
- ✅ FFmpeg backend: **ENABLED**
- ❌ Mock backend: **Auto-disabled**
- ❌ Tests: **OFF** (must explicitly enable)
- ❌ Coverage: **OFF** (must explicitly enable)
- ❌ Examples: **OFF** (must explicitly enable)

### Build FFmpeg Backend + Tests

```bash
cmake .. -DENABLE_FFMPEG_BACKEND=ON -DBUILD_TESTS=ON
```

### Build FFmpeg Backend + Tests + Coverage

```bash
cmake .. -DENABLE_FFMPEG_BACKEND=ON -DBUILD_TESTS=ON -DENABLE_COVERAGE=ON
```

### Build FFmpeg Backend + Example

```bash
cmake .. \
  -DENABLE_FFMPEG_BACKEND=ON \
  -DBUILD_FFMPEG_AUDIO_EXAMPLE=ON
```

**Note**: When you enable a specific example (e.g., `BUILD_FFMPEG_AUDIO_EXAMPLE=ON`), `BUILD_EXAMPLES` is automatically enabled. You don't need to pass both options.

### Build Only Apple Backend (macOS/iOS only)

```bash
cmake .. -DENABLE_APPLE_BACKEND=ON
```

**Result:**
- ✅ Apple backend: **ENABLED**
- ❌ Mock backend: **Auto-disabled**
- ❌ Tests: **OFF** (must explicitly enable)
- ❌ Coverage: **OFF** (must explicitly enable)
- ❌ Examples: **OFF** (must explicitly enable)

### Build Apple Backend + Tests + Example

```bash
cmake .. \
  -DENABLE_APPLE_BACKEND=ON \
  -DBUILD_TESTS=ON \
  -DBUILD_APPLE_AUDIO_EXAMPLE=ON
```

**Note**: `BUILD_EXAMPLES` is automatically enabled when you enable a specific example.

## Understanding the Build System

### Backend Selection Logic

**Key Rule**: When you enable a non-Mock backend, Mock backend is **automatically disabled**.

- `cmake ..` → Mock backend only
- `cmake .. -DENABLE_FFMPEG_BACKEND=ON` → FFmpeg only (Mock auto-disabled)
- `cmake .. -DENABLE_APPLE_BACKEND=ON` → Apple only (Mock auto-disabled)

### Test Building Logic

**Default behavior (Mock backend only):**
- Tests are **ON by default**

**When non-Mock backend is enabled:**
- Tests are **OFF by default**
- Must explicitly enable: `-DBUILD_TESTS=ON`

Tests are only built if:
1. `BUILD_TESTS=ON` AND
2. The corresponding backend is enabled (`ENABLE_<BACKEND>=ON`)

### Coverage Logic

**Default behavior (Mock backend only):**
- Coverage is **ON by default**

**When non-Mock backend is enabled:**
- Coverage is **OFF by default**
- Must explicitly enable: `-DENABLE_COVERAGE=ON`

### Example Building Logic

**Examples are always OFF by default** (regardless of backend).

To build examples:
1. Enable specific example: `-DBUILD_FFMPEG_AUDIO_EXAMPLE=ON` or `-DBUILD_APPLE_AUDIO_EXAMPLE=ON`
   - **Note**: `BUILD_EXAMPLES` is automatically enabled when you enable a specific example
2. Ensure the corresponding backend is enabled

**Simplified usage:**
- ✅ `cmake .. -DENABLE_FFMPEG_BACKEND=ON -DBUILD_FFMPEG_AUDIO_EXAMPLE=ON` (BUILD_EXAMPLES auto-enabled)
- ❌ No need to also pass `-DBUILD_EXAMPLES=ON`

## Common Scenarios

### Scenario 1: Default Build (Mock Backend)

```bash
cmake ..
```

**Result:**
- ✅ Mock backend
- ✅ Mock backend tests
- ✅ Coverage enabled
- ❌ Examples (none for Mock)

### Scenario 2: FFmpeg Backend Development

```bash
cmake .. \
  -DCMAKE_BUILD_TYPE=Debug \
  -DENABLE_FFMPEG_BACKEND=ON \
  -DBUILD_TESTS=ON \
  -DENABLE_COVERAGE=ON
```

**Result:**
- ✅ FFmpeg backend
- ✅ FFmpeg backend tests
- ✅ Coverage enabled
- ❌ Mock backend (auto-disabled)
- ❌ Examples (not enabled)

### Scenario 3: FFmpeg Backend + Example

```bash
cmake .. \
  -DENABLE_FFMPEG_BACKEND=ON \
  -DBUILD_FFMPEG_AUDIO_EXAMPLE=ON
```

**Result:**
- ✅ FFmpeg backend
- ✅ FFmpeg audio example
- ✅ BUILD_EXAMPLES (auto-enabled)
- ❌ Tests (not enabled)
- ❌ Coverage (not enabled)
- ❌ Mock backend (auto-disabled)

**Note**: You don't need to pass `-DBUILD_EXAMPLES=ON` - it's automatically enabled when you enable a specific example.

### Scenario 4: Production Build (FFmpeg Only)

```bash
cmake .. \
  -DCMAKE_BUILD_TYPE=Release \
  -DENABLE_FFMPEG_BACKEND=ON
```

**Result:**
- ✅ FFmpeg backend only
- ❌ Tests (not needed for production)
- ❌ Coverage (not needed for production)
- ❌ Examples (not needed for production)
- ❌ Mock backend (auto-disabled)

### Scenario 5: All Backends + Tests

```bash
cmake .. \
  -DENABLE_MOCK_BACKEND=ON \
  -DENABLE_FFMPEG_BACKEND=ON \
  -DENABLE_APPLE_BACKEND=ON \
  -DBUILD_TESTS=ON
```

**Note**: You must explicitly enable Mock backend if you want it alongside others.

## Summary Table

| Command | Mock | FFmpeg | Tests | Coverage | Examples |
|---------|------|--------|-------|----------|----------|
| `cmake ..` | ✅ | ❌ | ✅ | ✅ | ❌ |
| `cmake .. -DENABLE_FFMPEG_BACKEND=ON` | ❌ (auto) | ✅ | ❌ | ❌ | ❌ |
| `cmake .. -DENABLE_FFMPEG_BACKEND=ON -DBUILD_TESTS=ON` | ❌ (auto) | ✅ | ✅ | ❌ | ❌ |
| `cmake .. -DENABLE_FFMPEG_BACKEND=ON -DBUILD_TESTS=ON -DENABLE_COVERAGE=ON` | ❌ (auto) | ✅ | ✅ | ✅ | ❌ |

## Troubleshooting

### Mock Backend Still Building

**Problem**: Mock backend is building even though you enabled another backend

**Solution**: This shouldn't happen - Mock is auto-disabled when another backend is enabled. Check your CMake cache:
```bash
rm -rf build CMakeCache.txt
cmake .. -DENABLE_FFMPEG_BACKEND=ON
```

### Tests Not Building

**Problem**: Tests aren't being built even though you enabled a backend

**Solution**: Tests are OFF by default when non-Mock backend is enabled. Explicitly enable them:
```bash
cmake .. -DENABLE_FFMPEG_BACKEND=ON -DBUILD_TESTS=ON
```

### Coverage Not Working

**Problem**: Coverage isn't being generated

**Solution**: Coverage is OFF by default when non-Mock backend is enabled. Explicitly enable it:
```bash
cmake .. -DENABLE_FFMPEG_BACKEND=ON -DBUILD_TESTS=ON -DENABLE_COVERAGE=ON
```
