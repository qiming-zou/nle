import os

import numpy as np

from nle import _pynle
from nle._pynle.nethack import *  # noqa: F403

# TOOD: Don't use environment variables for this, add to nle.c instead.
os.environ["NETHACKOPTIONS"] = "nolegacy,nocmdassist"

DLPATH = os.path.join(os.path.dirname(_pynle.__file__), "libnethack.so")

DUNGEON_SHAPE = (21, 79)
BLSTATS_SHAPE = (23,)

OBSERVATION_DESC = {
    "glyphs": dict(shape=DUNGEON_SHAPE, dtype=np.int16),
    "chars": dict(shape=DUNGEON_SHAPE, dtype=np.uint8),
    "colors": dict(shape=DUNGEON_SHAPE, dtype=np.uint8),
    "specials": dict(shape=DUNGEON_SHAPE, dtype=np.uint8),
    "blstats": dict(shape=BLSTATS_SHAPE, dtype=np.int64),
}


class NLE:
    def __init__(
        self, observation_keys=OBSERVATION_DESC.keys(), copy=False,
    ):
        self._copy = copy
        self._pynle = _pynle.NLE(DLPATH)

        self._obs_buffers = {}

        for key in observation_keys:
            if key not in OBSERVATION_DESC:
                raise ValueError("Unknown observation '%s'" % key)
            self._obs_buffers[key] = np.zeros(**OBSERVATION_DESC[key])

        self._pynle.set_buffers(**self._obs_buffers)

        self._obs = tuple(self._obs_buffers[key] for key in observation_keys)
        if self._copy:
            self._step_return = lambda: tuple(o.copy() for o in self._obs)
        else:
            self._step_return = lambda: self._obs

    def step(self, action):
        self._pynle.step(action)
        return self._step_return(), self._pynle.done()

    def reset(self):
        self._pynle.reset()
        return self._step_return()