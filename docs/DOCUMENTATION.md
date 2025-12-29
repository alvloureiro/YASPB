# Documentation

This document provides information about the available documentation for the yaspb project.

## API Documentation

### API Analysis

See [API_ANALYSIS.md](API_ANALYSIS.md) for a detailed analysis of the API architecture, components, and relationships.

The API analysis covers:
- Interface design and patterns
- Component relationships
- Data structures and their usage
- Backend abstraction layer

### API Diagram

A PlantUML diagram is available at `docs/api-diagram.puml` showing the relationships between all API components.

**Generating the Diagram:**

```bash
# Install PlantUML (if not already installed)
# macOS: brew install plantuml
# Linux: apt-get install plantuml

# Generate PNG diagram
plantuml docs/api-diagram.puml

# Or use online tools like http://www.plantuml.com/plantuml/uml/
```

The diagram shows:
- All interfaces and their methods
- Data structures and their relationships
- Enums and their values
- Component dependencies and interactions

## Project Documentation

### Building

- [Building Guide](BUILDING.md) - Complete guide to building the project
- [Build Configurations](BUILD_CONFIGURATIONS.md) - Detailed build configuration options
- [Cross-Platform Building](CROSS_PLATFORM_BUILDING.md) - Cross-compilation instructions

### Usage

- [Usage Guide](USAGE.md) - How to use the library in your projects
- [Project Structure](PROJECT_STRUCTURE.md) - Project organization and structure

### Testing

- [Testing Guide](TESTING.md) - Testing and code coverage information

### Development

- [CLion Setup](CLION_SETUP.md) - Setting up the project in CLion IDE
- [RAII Review](RAII_REVIEW.md) - Review of RAII patterns used in the project

## Code Documentation

### Header Files

All public API headers in `include/playback/` are well-documented with:
- Class and interface descriptions
- Method documentation
- Parameter descriptions
- Return value documentation
- Usage examples where appropriate

### Source Files

Implementation files in `src/` include:
- Internal documentation for complex algorithms
- Backend-specific implementation notes
- Platform-specific considerations

## Generating Documentation

### Doxygen (if configured)

If Doxygen is configured for the project:

```bash
# Generate documentation
doxygen Doxyfile

# View documentation
open docs/html/index.html  # macOS
xdg-open docs/html/index.html  # Linux
```

## Contributing Documentation

When adding new features or making changes:

1. Update relevant documentation files
2. Add examples if introducing new APIs
3. Update diagrams if architecture changes
4. Keep documentation in sync with code changes

## Documentation Standards

- Use Markdown for all documentation files
- Follow the existing structure and style
- Include code examples where helpful
- Keep documentation up-to-date with code changes
- Use clear, concise language

