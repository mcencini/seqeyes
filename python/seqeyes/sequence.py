"""Pure-Python wrapper around the SeqEyes C++ core (_seqeyes_core)."""

from __future__ import annotations

import os
from dataclasses import dataclass, field
from typing import Optional, Tuple

import numpy as np
import numpy.typing as npt

from . import _seqeyes_core


def read_version(filepath: str | os.PathLike) -> Tuple[int, int]:
    """Read the Pulseq version from a ``.seq`` file without fully loading it.

    Parameters
    ----------
    filepath : str or os.PathLike
        Path to the Pulseq ``.seq`` file.

    Returns
    -------
    tuple of (int, int)
        ``(major, minor)`` version numbers extracted from the ``[VERSION]``
        section of the file.

    Raises
    ------
    RuntimeError
        If the file cannot be opened or the version section cannot be parsed.

    Examples
    --------
    >>> major, minor = read_version("path/to/sequence.seq")
    >>> print(f"Pulseq v{major}.{minor}")
    Pulseq v1.4
    """
    return _seqeyes_core.read_version(os.fspath(filepath))


@dataclass
class Block:
    """A single decoded sequence block.

    Attributes
    ----------
    duration_us : float
        Block duration in microseconds.
    rf_time : numpy.ndarray or None
        RF time samples (µs) relative to block start, or ``None`` if no RF
        event is present.
    rf_amp : numpy.ndarray or None
        RF amplitude samples (Hz), or ``None`` if no RF event is present.
    rf_phase : numpy.ndarray or None
        RF phase samples (rad), or ``None`` if no RF event is present.
    gx_time : numpy.ndarray or None
        Gx gradient time samples (µs), or ``None`` if no Gx event.
    gx_wave : numpy.ndarray or None
        Gx gradient waveform samples (Hz/m), or ``None`` if no Gx event.
    gy_time : numpy.ndarray or None
        Gy gradient time samples (µs), or ``None`` if no Gy event.
    gy_wave : numpy.ndarray or None
        Gy gradient waveform samples (Hz/m), or ``None`` if no Gy event.
    gz_time : numpy.ndarray or None
        Gz gradient time samples (µs), or ``None`` if no Gz event.
    gz_wave : numpy.ndarray or None
        Gz gradient waveform samples (Hz/m), or ``None`` if no Gz event.
    adc_num_samples : int or None
        Number of ADC samples, or ``None`` if no ADC event.
    adc_dwell : int or None
        ADC dwell time in nanoseconds, or ``None`` if no ADC event.
    adc_delay : int or None
        ADC delay in microseconds, or ``None`` if no ADC event.
    """

    duration_us: float = 0.0
    rf_time: Optional[npt.NDArray[np.float32]] = None
    rf_amp: Optional[npt.NDArray[np.float32]] = None
    rf_phase: Optional[npt.NDArray[np.float32]] = None
    gx_time: Optional[npt.NDArray[np.float32]] = None
    gx_wave: Optional[npt.NDArray[np.float32]] = None
    gy_time: Optional[npt.NDArray[np.float32]] = None
    gy_wave: Optional[npt.NDArray[np.float32]] = None
    gz_time: Optional[npt.NDArray[np.float32]] = None
    gz_wave: Optional[npt.NDArray[np.float32]] = None
    adc_num_samples: Optional[int] = None
    adc_dwell: Optional[int] = None
    adc_delay: Optional[int] = None


class Sequence:
    """Read and inspect a Pulseq ``.seq`` file.

    The ``Sequence`` object loads a Pulseq file and exposes its blocks and
    metadata through a simple Python API.  Arbitrary RF/gradient shapes are
    decompressed on demand when :meth:`get_block` is called.

    Parameters
    ----------
    filepath : str or os.PathLike
        Path to the Pulseq ``.seq`` file to load.

    Raises
    ------
    RuntimeError
        If the file cannot be opened, the version is unsupported, or the
        sequence data fails to load.

    Examples
    --------
    >>> from seqeyes import Sequence
    >>> seq = Sequence("gre.seq")
    >>> print(seq.num_blocks)
    1200
    >>> blk = seq.get_block(0)
    >>> print(blk.duration_us)
    3000.0
    """

    def __init__(self, filepath: str | os.PathLike) -> None:
        self._core = _seqeyes_core.PySequence(os.fspath(filepath))

    # ------------------------------------------------------------------
    # Properties
    # ------------------------------------------------------------------

    @property
    def num_blocks(self) -> int:
        """int: Total number of blocks in the sequence."""
        return self._core.num_blocks()

    @property
    def version(self) -> Tuple[int, int]:
        """tuple of (int, int): ``(major, minor)`` Pulseq format version."""
        return (self._core.version_major(), self._core.version_minor())

    @property
    def duration(self) -> float:
        """float: Approximate total sequence duration in microseconds.

        Computed as the sum of all block durations.  This iterates over all
        blocks and therefore has O(N) cost on first call.
        """
        total = 0.0
        for i in range(self.num_blocks):
            d = self._core.get_block(i)
            total += d["duration_us"]
        return total

    @property
    def definitions(self) -> dict:
        """dict: Sequence definitions from the ``[DEFINITIONS]`` section.

        Keys are definition names (strings); values are either a single
        ``float`` or a ``list`` of ``float`` for multi-value definitions.
        """
        return self._core.definitions()

    # ------------------------------------------------------------------
    # Methods
    # ------------------------------------------------------------------

    def get_block(self, idx: int) -> Block:
        """Decode and return a single sequence block.

        Parameters
        ----------
        idx : int
            Zero-based block index in the range ``[0, num_blocks)``.

        Returns
        -------
        Block
            Decoded block data including RF, gradient, and ADC events.

        Raises
        ------
        IndexError
            If *idx* is outside the valid range.

        Examples
        --------
        >>> blk = seq.get_block(0)
        >>> if blk.rf_amp is not None:
        ...     print("RF block, max amp:", blk.rf_amp.max())
        """
        try:
            raw = self._core.get_block(idx)
        except (IndexError, RuntimeError) as exc:
            raise IndexError(f"Block index {idx} out of range") from exc

        return Block(
            duration_us=raw["duration_us"],
            rf_time=raw.get("rf_time"),
            rf_amp=raw.get("rf_amp"),
            rf_phase=raw.get("rf_phase"),
            gx_time=raw.get("gx_time"),
            gx_wave=raw.get("gx_wave"),
            gy_time=raw.get("gy_time"),
            gy_wave=raw.get("gy_wave"),
            gz_time=raw.get("gz_time"),
            gz_wave=raw.get("gz_wave"),
            adc_num_samples=raw.get("adc_num_samples"),
            adc_dwell=raw.get("adc_dwell"),
            adc_delay=raw.get("adc_delay"),
        )
