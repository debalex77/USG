#!/usr/bin/env python3

"""Verificări tehnice pentru bazele incluse în pachetul USG Demo."""

from __future__ import annotations

import base64
import hashlib
import sqlite3
import sys
from pathlib import Path


def fail(message: str) -> None:
    raise RuntimeError(message)


def integrity_check(connection: sqlite3.Connection, label: str) -> None:
    rows = connection.execute("PRAGMA integrity_check").fetchall()
    if rows != [("ok",)]:
        fail(f"{label}: PRAGMA integrity_check a eșuat: {rows}")

    foreign_key_errors = connection.execute("PRAGMA foreign_key_check").fetchall()
    if foreign_key_errors:
        fail(f"{label}: PRAGMA foreign_key_check a găsit erori.")


def verify_empty_admin_password(stored_hash: str) -> bool:
    parts = stored_hash.split("$")
    if len(parts) != 4 or parts[0] != "pbkdf2-sha256":
        return False

    try:
        rounds = int(parts[1])
        salt = base64.b64decode(parts[2], validate=True)
        expected = base64.b64decode(parts[3], validate=True)
    except (ValueError, TypeError):
        return False

    # Aplicația aplică PBKDF2 peste reprezentarea hex a SHA-256(parolă).
    legacy_digest = hashlib.sha256(b"").hexdigest().encode("ascii")
    actual = hashlib.pbkdf2_hmac("sha256", legacy_digest, salt, rounds, len(expected))
    return actual == expected


def verify_main_database(path: Path) -> tuple[int, int, int]:
    if not path.is_file():
        fail(f"Baza principală demo lipsește: {path}")

    with sqlite3.connect(f"file:{path}?mode=ro", uri=True) as connection:
        integrity_check(connection, "Baza principală demo")

        required_tables = {"users", "patients", "orderEcho", "reportEcho", "databaseMetadata"}
        existing_tables = {
            row[0]
            for row in connection.execute(
                "SELECT name FROM sqlite_master WHERE type='table'"
            )
        }
        missing_tables = sorted(required_tables - existing_tables)
        if missing_tables:
            fail("Baza principală demo nu conține: " + ", ".join(missing_tables))

        admins = connection.execute(
            """
            SELECT COALESCE(password, ''), COALESCE(hash, '')
            FROM users
            WHERE name = 'admin' AND deletionMark = 0
            """
        ).fetchall()
        if len(admins) != 1:
            fail("Baza demo trebuie să conțină exact un utilizator activ admin.")

        legacy_password, stored_hash = admins[0]
        if legacy_password:
            fail("users.password pentru admin trebuie să fie gol.")
        if not verify_empty_admin_password(stored_hash):
            fail("Hash-ul utilizatorului admin nu corespunde parolei goale.")

        patients = connection.execute("SELECT COUNT(*) FROM patients").fetchone()[0]
        orders = connection.execute("SELECT COUNT(*) FROM orderEcho").fetchone()[0]
        reports = connection.execute("SELECT COUNT(*) FROM reportEcho").fetchone()[0]
        return patients, orders, reports


def verify_image_database(image_path: Path, main_path: Path) -> tuple[int, int]:
    if not image_path.is_file():
        return 0, 0

    with sqlite3.connect(f"file:{image_path}?mode=ro", uri=True) as connection:
        integrity_check(connection, "Baza de imagini demo")

        has_table = connection.execute(
            """
            SELECT COUNT(*)
            FROM sqlite_master
            WHERE type = 'table' AND name = 'imagesReports'
            """
        ).fetchone()[0]
        if has_table != 1:
            fail("Baza de imagini demo nu conține tabela imagesReports.")

        connection.execute("ATTACH DATABASE ? AS main_demo", (str(main_path),))
        missing = connection.execute(
            """
            SELECT COUNT(*)
            FROM imagesReports AS image
            LEFT JOIN main_demo.patients AS patient ON patient.id = image.patient_id
            LEFT JOIN main_demo.orderEcho AS document_order ON document_order.id = image.id_orderEcho
            LEFT JOIN main_demo.reportEcho AS document_report ON document_report.id = image.id_reportEcho
            WHERE patient.id IS NULL
               OR document_order.id IS NULL
               OR document_report.id IS NULL
            """
        ).fetchone()[0]
        if missing:
            fail(
                "Baza de imagini demo conține "
                f"{missing} rânduri care nu corespund bazei principale demo."
            )

        rows = connection.execute("SELECT COUNT(*) FROM imagesReports").fetchone()[0]
        blob_slots = connection.execute(
            """
            SELECT
                SUM(image_1 IS NOT NULL AND length(image_1) > 0) +
                SUM(image_2 IS NOT NULL AND length(image_2) > 0) +
                SUM(image_3 IS NOT NULL AND length(image_3) > 0) +
                SUM(image_4 IS NOT NULL AND length(image_4) > 0) +
                SUM(image_5 IS NOT NULL AND length(image_5) > 0)
            FROM imagesReports
            """
        ).fetchone()[0] or 0
        return rows, blob_slots


def main() -> int:
    if len(sys.argv) not in (2, 3):
        print(
            f"Utilizare: {Path(sys.argv[0]).name} BAZA_PRINCIPALA [BAZA_IMAGINI]",
            file=sys.stderr,
        )
        return 2

    main_path = Path(sys.argv[1]).resolve()
    image_path = Path(sys.argv[2]).resolve() if len(sys.argv) == 3 else None

    patients, orders, reports = verify_main_database(main_path)
    print(
        "Baza principală demo este validă: "
        f"{patients} pacienți, {orders} comenzi, {reports} rapoarte; "
        "admin folosește parola goală."
    )

    if image_path:
        rows, blob_slots = verify_image_database(image_path, main_path)
        print(
            "Baza de imagini demo este compatibilă: "
            f"{rows} rânduri, {blob_slots} imagini BLOB."
        )

    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except (RuntimeError, sqlite3.Error) as error:
        print(f"Eroare: {error}", file=sys.stderr)
        raise SystemExit(1)
