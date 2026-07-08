export function euclid(k: number, n: number): number[] {
  const p: number[] = [];
  for (let i = 0; i < n; i++) {
    const cur = Math.floor((i * k) / n);
    const prev = Math.floor(((i - 1) * k) / n);
    p.push(cur !== prev ? 1 : 0);
  }
  if (k > 0) p[0] = 1;
  return p;
}

export function rotArr<T>(a: T[], r: number): T[] {
  const n = a.length;
  return a.map((_, i) => a[(((i - r) % n) + n) % n]);
}

export function gcd(a: number, b: number): number {
  let x = Math.abs(a);
  let y = Math.abs(b);
  while (y) {
    [x, y] = [y, x % y];
  }
  return x;
}

export function lcm(a: number, b: number): number {
  if (a === 0 || b === 0) return 0;
  return Math.abs((a / gcd(a, b)) * b);
}
