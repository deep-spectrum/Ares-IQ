from rich.progress import Progress, TextColumn, TaskProgressColumn, TimeElapsedColumn, BarColumn
import datetime as dt


class CaptureProgress:
    """Implementation of a progress bar."""

    def __init__(self, captures: int, samples_per_capture: int, hide: bool = False):
        """Initializes the progress bar.

        Args:
            captures: The total number of captures to collect.
            samples_per_capture: The number of samples per a capture.
            hide: If `True`, hide the progress bar.
        """
        if hide:
            self._hide = True
            return
        self._hide = False
        self._samples_captured = 0
        self._samples_per_capture = samples_per_capture
        self._progress = Progress(TextColumn("Capturing..."),
                                  BarColumn(),
                                  TaskProgressColumn(),
                                  TextColumn("[magenta]([progress.description]{task.description})"),
                                  TimeElapsedColumn())
        self._task = self._progress.add_task("[magenta]0.0 megasamples/second", total=captures * samples_per_capture)
        self._start = None

    def __enter__(self):
        """Context manager open. This starts the progress bar."""
        if not self._hide:
            self._progress.start()
            self._start = dt.datetime.now()
        return self

    def __exit__(self, exc_type, exc_val, exc_tb):
        """Context manager close. This stops the progress bar."""
        if not self._hide:
            self._progress.stop()

    def update(self):
        """Push an update to the progress bar."""
        if self._hide:
            return
        self._samples_captured += self._samples_per_capture
        time_diff = (dt.datetime.now() - self._start).total_seconds()
        rate = f"{self._samples_captured / time_diff / 1e6:.2f} megasamples/second"
        self._progress.update(self._task, completed=self._samples_captured, description=rate)
