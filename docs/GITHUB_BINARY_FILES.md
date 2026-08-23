# GitHub binary-file guidance

The current CAD files are below GitHub's 100 MB single-file hard limit, but STEP/STL/glTF binaries can grow repository history quickly. For the first upload, the included snapshot can be committed normally. Before many CAD revisions accumulate, consider Git LFS for heavy binary formats or keep generated exports in tagged releases while versioning parametric source and critical manufacturing exports in Git.
