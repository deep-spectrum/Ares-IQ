PACKAGE_DIR=src/ares_iq

.phony: typecheck
typecheck:
	python3 -m mypy $(PACKAGE_DIR)

