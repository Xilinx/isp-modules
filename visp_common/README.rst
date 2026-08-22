.. SPDX-License-Identifier: MIT

Shared VISP sources
===================

``visp_common`` is the canonical implementation for ISP controls, CamDevice
operations, and their shared interface headers.  The unified LIMO/LILO, MIMO,
mailbox, and MIMO-video Makefiles select the appropriate wrappers under
``.kbuild`` and compile these sources with the required variant definition.
Variant conditionals are kept local to the declarations, packet fields, or
operations that actually differ; complete per-variant implementations must
not be duplicated inside a common source or header.

The unified ``visp`` module handles LIMO and LILO at runtime and uses the
``.kbuild/visp`` wrappers for both modes.  There is no standalone LILO wrapper
tree.  LIMO/LILO differences in shared code must use ``isp_mode`` rather than a
compile-time LILO definition.

Remove the compatibility copies after downstream builds no longer reference
their legacy paths.
