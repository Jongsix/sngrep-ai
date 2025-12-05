#!/usr/bin/env python3
"""
Simple AI-powered SIP traffic analyzer

This script demonstrates how to consume JSON output from sngrep --ai-agent mode
and perform basic analysis on SIP traffic.

Usage:
    sngrep --ai-agent -I capture.pcap 2>/dev/null | python examples/ai_analyzer.py
    sngrep --ai-agent -d eth0 2>/dev/null | python examples/ai_analyzer.py

Note: Redirect stderr (2>/dev/null) to filter out debug messages from sngrep
"""

import sys
import json
import re
from collections import defaultdict
from datetime import datetime


class SIPAnalyzer:
    def __init__(self):
        self.stats = {
            'total_messages': 0,
            'total_batches': 0,
            'methods': defaultdict(int),
            'response_codes': defaultdict(int),
            'call_ids': set(),
            'endpoints': defaultdict(int),
        }

    def analyze_batch(self, batch):
        """Analyze a single batch of SIP messages"""
        self.stats['total_batches'] += 1
        self.stats['total_messages'] += batch['packet_count']

        print(f"\n{'='*80}")
        print(f"Batch {batch['batch_id']} @ {batch['timestamp']}")
        print(f"Messages: {batch['packet_count']}")
        print(f"{'='*80}")

        for msg in batch['messages']:
            self.analyze_message(msg)

    def analyze_message(self, msg):
        """Analyze a single SIP message"""
        # Track call IDs
        self.stats['call_ids'].add(msg['call_id'])

        # Track methods and response codes
        if msg['method']:
            self.stats['methods'][msg['method']] += 1
            msg_type = msg['method']
        else:
            self.stats['response_codes'][msg['response_code']] += 1
            msg_type = f"{msg['response_code']}"

        # Track endpoints
        self.stats['endpoints'][msg['src_addr']] += 1
        self.stats['endpoints'][msg['dst_addr']] += 1

        # Print message summary
        print(f"\n  [{msg['index']}] {msg['timestamp']}")
        print(f"  Type: {msg_type}")
        print(f"  Call-ID: {msg['call_id']}")
        print(f"  From: {msg['from']} -> To: {msg['to']}")
        print(f"  Path: {msg['src_addr']} -> {msg['dst_addr']}")
        print(f"  CSeq: {msg['cseq']}")

        # Detect potential issues
        self.detect_anomalies(msg)

    def detect_anomalies(self, msg):
        """Simple anomaly detection"""
        anomalies = []

        # Check for authentication failures
        if msg['response_code'] in [401, 403, 407]:
            anomalies.append("Authentication failure")

        # Check for call failures
        if msg['response_code'] and msg['response_code'] >= 400:
            anomalies.append(f"Error response: {msg['response_code']}")

        # Check for timeouts
        if msg['response_code'] == 408:
            anomalies.append("Request timeout")

        if anomalies:
            print(f"  ⚠️  ANOMALIES: {', '.join(anomalies)}")

    def print_summary(self):
        """Print overall statistics"""
        print(f"\n\n{'='*80}")
        print("ANALYSIS SUMMARY")
        print(f"{'='*80}")
        print(f"Total Batches: {self.stats['total_batches']}")
        print(f"Total Messages: {self.stats['total_messages']}")
        print(f"Unique Calls: {len(self.stats['call_ids'])}")

        print(f"\n📊 SIP Methods:")
        for method, count in sorted(self.stats['methods'].items(), key=lambda x: x[1], reverse=True):
            print(f"  {method}: {count}")

        print(f"\n📊 Response Codes:")
        for code, count in sorted(self.stats['response_codes'].items(), key=lambda x: x[1], reverse=True):
            print(f"  {code}: {count}")

        print(f"\n📊 Top Endpoints:")
        for endpoint, count in sorted(self.stats['endpoints'].items(), key=lambda x: x[1], reverse=True)[:10]:
            print(f"  {endpoint}: {count} messages")

        print(f"\n{'='*80}\n")


def main():
    analyzer = SIPAnalyzer()

    try:
        # Read all input
        input_data = sys.stdin.read()

        # Find the start of JSON objects (look for lines starting with '{')
        # Remove any prefix debug output like "Dialog count: 0"
        json_start = input_data.find('{')
        if json_start > 0:
            input_data = input_data[json_start:]

        # Parse each JSON object (batches are separated by newlines at root level)
        decoder = json.JSONDecoder()
        idx = 0
        while idx < len(input_data):
            # Skip whitespace
            while idx < len(input_data) and input_data[idx] in ' \t\n\r':
                idx += 1

            if idx >= len(input_data):
                break

            try:
                batch, end_idx = decoder.raw_decode(input_data, idx)
                # Verify it's a valid batch dictionary
                if isinstance(batch, dict) and 'batch_id' in batch and 'messages' in batch:
                    analyzer.analyze_batch(batch)
                idx += end_idx
            except (json.JSONDecodeError, ValueError):
                # Skip to next potential JSON start
                idx += 1

    except KeyboardInterrupt:
        print("\n\nInterrupted by user", file=sys.stderr)

    finally:
        analyzer.print_summary()


if __name__ == '__main__':
    main()
