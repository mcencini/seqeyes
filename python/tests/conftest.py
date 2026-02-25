"""pytest fixtures for seqeyes tests."""

import textwrap
import pytest


@pytest.fixture()
def minimal_seq_file(tmp_path):
    """Generate a minimal valid Pulseq v1.4 .seq file with one RF block.

    The file contains:
    - A [VERSION] section (v1.4)
    - A [DEFINITIONS] section with required raster times
    - A [BLOCKS] section with a single block containing an RF event
    - An [RF] library entry
    - A [SHAPES] section with trivial shapes

    Returns
    -------
    pathlib.Path
        Path to the generated ``.seq`` file.
    """
    content = textwrap.dedent("""\
        # Pulseq sequence file
        # Created for testing

        [VERSION]
        major 1
        minor 4
        revision 1

        [DEFINITIONS]
        AdcRasterTime 1e-07
        GradientRasterTime 1e-05
        RadiofrequencyRasterTime 1e-06
        BlockDurationRaster 1e-05

        [BLOCKS]
        1 1 0 0 0 0 0 200

        [RF]
        1 500 1 2 0 0 0 0 0 0

        [SHAPES]
        shape_id 1
        num_samples 2
        0
        0
        shape_id 2
        num_samples 2
        0
        0

        [SIGNATURE]
        Type notset
        Hash notset

    """)

    seq_file = tmp_path / "test.seq"
    seq_file.write_text(content)
    return seq_file
