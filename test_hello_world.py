import hashlib
import logging
import pytest
from pytest_embedded_qemu.app import QemuApp
from pytest_embedded_qemu.dut import QemuDut

def verify_elf_sha256_embedding(app: QemuApp, sha256_reported: str) -> None:
    sha256 = hashlib.sha256()
    with open(app.elf_file, 'rb') as f:
        sha256.update(f.read())
    sha256_expected = sha256.hexdigest()

    logging.info(f'ELF file SHA256: {sha256_expected}')
    logging.info(f'ELF file SHA256 (reported by the app): {sha256_reported}')

    if not sha256_expected.startswith(sha256_reported):
        raise ValueError('ELF file SHA256 mismatch')


@pytest.mark.esp32
@pytest.mark.qemu
def test_hello_world_qemu(app: QemuApp, dut: QemuDut) -> None:
    # cocokkan SHA256 yang dilaporkan firmware
    sha256_reported = (
        dut.expect(r'ELF file SHA256:\s+([a-f0-9]+)').group(1).decode('utf-8')
    )
    verify_elf_sha256_embedding(app, sha256_reported)

    # cocokkan pesan hello
    dut.expect("Hello world!")
