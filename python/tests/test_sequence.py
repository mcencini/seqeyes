"""Tests for seqeyes.Sequence and seqeyes.Block."""

import numpy as np
import pytest
from seqeyes import Sequence, Block


# ---------------------------------------------------------------------------
# Basic loading
# ---------------------------------------------------------------------------

def test_sequence_loads(minimal_seq_file):
    """Sequence should load without raising an exception."""
    seq = Sequence(minimal_seq_file)
    assert seq is not None


def test_sequence_num_blocks_positive(minimal_seq_file):
    """A loaded sequence must have at least one block."""
    seq = Sequence(minimal_seq_file)
    assert seq.num_blocks > 0


def test_sequence_version(minimal_seq_file):
    """version property should return (1, 4) for the minimal test file."""
    seq = Sequence(minimal_seq_file)
    major, minor = seq.version
    assert major == 1
    assert minor == 4


def test_sequence_definitions(minimal_seq_file):
    """definitions property should return a dict."""
    seq = Sequence(minimal_seq_file)
    defs = seq.definitions
    assert isinstance(defs, dict)


# ---------------------------------------------------------------------------
# Block extraction
# ---------------------------------------------------------------------------

def test_get_block_returns_block(minimal_seq_file):
    """get_block should return a Block instance."""
    seq = Sequence(minimal_seq_file)
    blk = seq.get_block(0)
    assert isinstance(blk, Block)


def test_block_has_duration(minimal_seq_file):
    """Block duration_us should be a positive float."""
    seq = Sequence(minimal_seq_file)
    blk = seq.get_block(0)
    assert isinstance(blk.duration_us, float)
    assert blk.duration_us > 0


def test_block_rf_arrays_are_numpy(minimal_seq_file):
    """RF amplitude/phase/time should be numpy arrays when an RF event is present."""
    seq = Sequence(minimal_seq_file)
    blk = seq.get_block(0)
    # The minimal file has an RF block
    if blk.rf_amp is not None:
        assert isinstance(blk.rf_amp, np.ndarray)
        assert isinstance(blk.rf_phase, np.ndarray)
        assert isinstance(blk.rf_time, np.ndarray)


def test_block_gradient_none_when_absent(minimal_seq_file):
    """Gradient fields should be None for a block with no gradient events."""
    seq = Sequence(minimal_seq_file)
    blk = seq.get_block(0)
    # Minimal file has no gradients in the only block
    assert blk.gx_time is None
    assert blk.gx_wave is None
    assert blk.gy_time is None
    assert blk.gy_wave is None
    assert blk.gz_time is None
    assert blk.gz_wave is None


def test_block_adc_none_when_absent(minimal_seq_file):
    """ADC fields should be None for a block with no ADC event."""
    seq = Sequence(minimal_seq_file)
    blk = seq.get_block(0)
    # Minimal file has no ADC
    assert blk.adc_num_samples is None
    assert blk.adc_dwell is None
    assert blk.adc_delay is None


# ---------------------------------------------------------------------------
# Error handling
# ---------------------------------------------------------------------------

def test_sequence_nonexistent_file(tmp_path):
    """Sequence should raise RuntimeError for a missing file."""
    with pytest.raises((RuntimeError, OSError, FileNotFoundError)):
        Sequence(str(tmp_path / "missing.seq"))


def test_get_block_out_of_range(minimal_seq_file):
    """get_block should raise IndexError for out-of-range index."""
    seq = Sequence(minimal_seq_file)
    with pytest.raises((IndexError, RuntimeError)):
        seq.get_block(seq.num_blocks + 100)
