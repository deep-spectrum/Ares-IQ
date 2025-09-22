PACKAGE_DIR=src/iq_capture

.phony: typecheck
typecheck:
	python3 -m mypy $(PACKAGE_DIR)

