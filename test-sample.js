const fs = require('fs');
const path = require('path');

// Helper to decode y values from any base
function decodeValue(value, base) {
    return BigInt(parseInt(value, base));
}

// Manual Lagrange interpolation at x=0 to find the constant term c
function lagrangeConstant(points, k) {
    let c = 0n;
    for (let i = 0; i < k; i++) {
        let xi = BigInt(points[i].x);
        let yi = points[i].y;
        let numerator = 1n;
        let denominator = 1n;
        for (let j = 0; j < k; j++) {
            if (i === j) continue;
            let xj = BigInt(points[j].x);
            numerator *= -xj;
            denominator *= (xi - xj);
        }
        // Compute term: yi * numerator / denominator
        // Use division for BigInt (no modulus required as per constraints)
        let term = yi * numerator / denominator;
        c += term;
    }
    return c;
}

// Read and process a test case from a file
function processTestCase(filename) {
    const inputPath = path.join(__dirname, filename);
    if (!fs.existsSync(inputPath)) {
        console.error(`File not found: ${filename}`);
        return null;
    }
    const data = JSON.parse(fs.readFileSync(inputPath, 'utf8'));
    const { n, k } = data.keys;
    // Get first k points (as per assignment, n >= k)
    const points = Object.entries(data)
        .filter(([key]) => !isNaN(Number(key)))
        .map(([key, { base, value }]) => ({
            x: Number(key),
            y: decodeValue(value, Number(base))
        }))
        .slice(0, k);
    // Find the secret (constant term)
    const secret = lagrangeConstant(points, k);
    return secret.toString();
}

// Print secret for both testcases
const secret1 = processTestCase('input1.json');
const secret2 = processTestCase('input2.json');
console.log('Secret for testcase 1:', secret1 !== null ? secret1 : 'input1.json not found');
console.log('Secret for testcase 2:', secret2 !== null ? secret2 : 'input2.json not found');
