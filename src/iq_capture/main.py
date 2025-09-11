import typer


app = typer.Typer()


@app.command()
def capture():
    pass


@app.command(name='set-platform')
def set_platform():
    pass


def main():
    app()


if __name__ == "__main__":
    main()
