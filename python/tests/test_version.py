"""Tests for seqeyes.read_version()."""

import pytest
from seqeyes import read_version


def test_read_version_returns_tuple(minimal_seq_file):
    """read_version should return a 2-tuple of ints."""
    result = read_version(minimal_seq_file)
    assert isinstance(result, tuple)
    assert len(result) == 2


def test_read_version_correct_values(minimal_seq_file):
    """read_version should return (1, 4) for the minimal test file."""
    major, minor = read_version(minimal_seq_file)
    assert major == 1
    assert minor == 4


def test_read_version_accepts_pathlike(minimal_seq_file):
    """read_version should accept a pathlib.Path object."""
    import pathlib
    result = read_version(pathlib.Path(minimal_seq_file))
    assert result[0] == 1


def test_read_version_nonexistent_file(tmp_path):
    """read_version should raise RuntimeError for a missing file."""
    with pytest.raises((RuntimeError, OSError)):
        read_version(str(tmp_path / "does_not_exist.seq"))
