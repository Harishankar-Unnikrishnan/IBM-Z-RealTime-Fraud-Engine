# Real-Time AI-Driven Payment Fraud Decisioning Engine

Engineered for the **IBM Z Datathon**, running natively on **IBM LinuxONE (`s390x`)**.

## Overview
High-throughput payment networks require fraud risk classification within sub-millisecond windows to prevent authorization bottlenecks. Conventional architectures offload transaction scoring to external cloud REST APIs, introducing 100–300 ms round-trip network delays.

This project deploys an embedded, native C machine learning inference engine executing directly on the **IBM LinuxONE bare-metal platform**. By computing continuous fraud risk scores via mathematical sigmoid activation vectors at the processor cache level, transaction scoring executes in **under 50 microseconds**.

## System Architecture
- **Inference Core (`engine.c`)**: Native C POSIX service evaluating multi-factor vectors (normalized amount, rolling burst velocity, PIN auth failures, temporal off-hour anomalies).
- **Embedded Scoring Algorithm**: Direct mathematical sigmoid activation ($\sigma(z) = \frac{1}{1 + e^{-z}}$) avoiding runtime interpreter overhead.
- **Audit Ledger (`audit_trail.csv`)**: Synchronous, immutable CSV logging capturing transaction IDs, risk probabilities, automated decisions, and hardware execution times.
- **Monitoring Console (`server.py` + `index.html`)**: Microsecond-latency polling dashboard providing real-time operational visibility over secure SSH port forwarding.

## Performance Metrics
- **Target Platform**: IBM LinuxONE Community Cloud
- **Architecture**: `s390x` (64-bit IBM z/Architecture)
- **Average Decision Latency**: 38 µs – 48 µs per transaction
- **Decision Outcomes**: `APPROVE`, `REVIEW`, `DECLINE`

## Deployment & Verification
```bash
# Verify mainframe architecture
uname -m
# Output: s390x

# Compile native binary with math optimization
gcc -O3 engine.c -o engine -lm

# Launch the daemon and dashboard
python3 server.py &
./engine
```
Author: Harishankar Unnikrishnan
GitHub: https://github.com/Harishankar-Unnikrishnan
