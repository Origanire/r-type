# Project documentation

The project currently provides a networked movement prototype. Combat is planned.
Build and quick-start commands are in the repository [README](../README.md).

- [Network documentation: protocol, usage, APIs and errors](RESEAU.md)
- [Architecture overview](architecture/overview.md)
- [Engine modules](architecture/engine-modules.md)
- [Entity Component System](architecture/ecs.md)
- [Rendering](architecture/rendering.md)
- [Coding conventions](conventions.md)
- [Add a component](guides/add-a-component.md)
- [Add a system](guides/add-a-system.md)
- [Implementation roadmap](roadmap.md)

## Preview the documentation site

From the repository root, use a dedicated environment:

```sh
python3 -m venv .venv
. .venv/bin/activate
python -m pip install mkdocs-material pymdown-extensions
mkdocs serve
```

MkDocs reads the Markdown files in `docs/`. The documentation site has not been deployed.
The project subject PDFs are retained in this directory as reference material.
