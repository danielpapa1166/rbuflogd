#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
DAEMON_BIN="${ROOT_DIR}/build/rbuflogd"
PRODUCER_BIN="${ROOT_DIR}/build/rbuflogd_producer_test"
LOG_FILE="${ROOT_DIR}/rbuflogd.log"

PRODUCERS="${PRODUCERS:-100}"
MSGS_PER_PRODUCER="${MSGS_PER_PRODUCER:-100}"
DRAIN_TIMEOUT_SEC="${DRAIN_TIMEOUT_SEC:-10}"

if [[ ! -x "${DAEMON_BIN}" || ! -x "${PRODUCER_BIN}" ]]; then
  echo "Build artifacts not found. Run: cmake -S . -B build && cmake --build build" >&2
  exit 1
fi

TMP_DIR="$(mktemp -d)"
DAEMON_PID=""

cleanup() {
  if [[ -n "${DAEMON_PID}" ]] && kill -0 "${DAEMON_PID}" 2>/dev/null; then
    kill -TERM "${DAEMON_PID}" 2>/dev/null || true
    wait "${DAEMON_PID}" 2>/dev/null || true
  fi
  rm -rf "${TMP_DIR}"
}
trap cleanup EXIT

echo "[1/5] Starting daemon"
: > "${LOG_FILE}"
"${DAEMON_BIN}" >"${TMP_DIR}/daemon.out" 2>&1 &
DAEMON_PID="$!"

# Give daemon a brief moment to initialize shared memory.
sleep 0.3

echo "[2/5] Sending logs with ${PRODUCERS} concurrent producers x ${MSGS_PER_PRODUCER} messages"
producer_pids=()
for p in $(seq 0 $((PRODUCERS - 1))); do
  (
    producer_name="$(printf "p%07d" "${p}")"
    category="$(printf "c%07d" "${p}")"
    expected_file="${TMP_DIR}/expected_${p}.txt"

    for i in $(seq 0 $((MSGS_PER_PRODUCER - 1))); do
      msg_id="$(printf "p%07d-%05d" "${p}" "${i}")"
      printf '%s\n' "${msg_id}" >> "${expected_file}"
      "${PRODUCER_BIN}" -p "${producer_name}" -c "${category}" -m "${msg_id}" >/dev/null 2>&1 || true
    done
  ) &
  producer_pids+=("$!")
done

for pid in "${producer_pids[@]}"; do
  wait "${pid}"
done

expected_total=$((PRODUCERS * MSGS_PER_PRODUCER))
cat "${TMP_DIR}"/expected_*.txt | sort > "${TMP_DIR}/expected_all.txt"

echo "[3/5] Waiting for daemon to drain ring buffer"
last_count=-1
stable_ticks=0
end_epoch=$((SECONDS + DRAIN_TIMEOUT_SEC))

while (( SECONDS < end_epoch )); do
  line_count="$(wc -l < "${LOG_FILE}" 2>/dev/null || echo 0)"

  if [[ "${line_count}" -eq "${last_count}" ]]; then
    stable_ticks=$((stable_ticks + 1))
  else
    stable_ticks=0
    last_count="${line_count}"
  fi

  # Consider drained when count stays stable for ~0.6s.
  if (( stable_ticks >= 3 )); then
    break
  fi

  sleep 0.2
done

echo "[4/5] Stopping daemon"
kill -TERM "${DAEMON_PID}" 2>/dev/null || true
wait "${DAEMON_PID}" 2>/dev/null || true
DAEMON_PID=""

echo "[5/5] Checking consistency"
grep -oE 'p[0-9]{7}-[0-9]{5}' "${LOG_FILE}" | sort > "${TMP_DIR}/actual_all.txt" || true

comm -23 "${TMP_DIR}/expected_all.txt" "${TMP_DIR}/actual_all.txt" > "${TMP_DIR}/missing.txt" || true
comm -13 "${TMP_DIR}/expected_all.txt" "${TMP_DIR}/actual_all.txt" > "${TMP_DIR}/extra.txt" || true
uniq -d "${TMP_DIR}/actual_all.txt" > "${TMP_DIR}/duplicates.txt" || true

missing_count="$(wc -l < "${TMP_DIR}/missing.txt")"
extra_count="$(wc -l < "${TMP_DIR}/extra.txt")"
dup_count="$(wc -l < "${TMP_DIR}/duplicates.txt")"
actual_total="$(wc -l < "${TMP_DIR}/actual_all.txt")"

echo ""
echo "Stress test summary"
echo "- expected messages: ${expected_total}"
echo "- parsed messages:   ${actual_total}"
echo "- missing:           ${missing_count}"
echo "- extra:             ${extra_count}"
echo "- duplicates:        ${dup_count}"

if (( missing_count == 0 && extra_count == 0 && dup_count == 0 )); then
  echo "PASS: no inconsistencies detected"
  exit 0
fi

echo "FAIL: inconsistencies detected"
if (( missing_count > 0 )); then
  echo "First missing IDs:"
  head -n 10 "${TMP_DIR}/missing.txt"
fi
if (( extra_count > 0 )); then
  echo "First extra IDs:"
  head -n 10 "${TMP_DIR}/extra.txt"
fi
if (( dup_count > 0 )); then
  echo "First duplicate IDs:"
  head -n 10 "${TMP_DIR}/duplicates.txt"
fi

exit 2
