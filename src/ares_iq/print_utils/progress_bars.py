from rich.progress import Progress, TextColumn, TaskProgressColumn, TimeElapsedColumn, BarColumn
import datetime as dt


class CaptureProgress:
    def __init__(self, captures: int, samples_per_capture: int):
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
        self._progress.start()
        self._start = dt.datetime.now()
        return self

    def __exit__(self, exc_type, exc_val, exc_tb):
        self._progress.stop()

    def update(self):
        self._samples_captured += self._samples_per_capture
        time_diff = (dt.datetime.now() - self._start).total_seconds()
        rate = f"{self._samples_captured / time_diff / 1e6:.2f} megasamples/second"
        self._progress.update(self._task, completed=self._samples_captured, description=rate)
