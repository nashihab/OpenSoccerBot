# Battery configuration guide

Battery choice should start from the **motor voltage and current requirements**, then work backward into the driver, fuse, wire gauge and regulator.

## Voltage basics

| Pack | Cells | Nominal voltage | Full charge voltage |
|---|---:|---:|---:|
| 2S | 2 in series | 7.4 V | 8.4 V |
| 3S | 3 in series | 11.1 V | 12.6 V |
| 4S | 4 in series | 14.8 V | 16.8 V |

Use a pack voltage that is appropriate for the **motor's rated voltage**. Do not pick a higher voltage just because it gives more speed.
## Important for the TB6612FNG builds

The TB6612FNG operating VM range tops out at 13.5 V. A 4S LiPo/Li-ion pack reaches 16.8 V when fully charged, so **do not use 4S with the TB6612FNG used in this repository**. Stay at 2S or 3S unless the motor-driver stage is replaced with a driver rated for the higher pack voltage.

Reference: https://toshiba.semicon-storage.com/info/docget.jsp?did=10660


## Series vs parallel

- **S (series)** increases voltage.
- **P (parallel)** increases capacity and available current while keeping the series voltage the same.

### 2S1P

Two cells in series:

```text
Pack − ──[ Cell 1 ]──[ Cell 2 ]── Pack +

7.4 V nominal / 8.4 V full
```

### 2S2P

Four cells arranged as two cells in parallel, then two such groups in series:

```text
[Cell || Cell] ── [Cell || Cell]
      1P group         1P group

7.4 V nominal
Capacity ≈ 2 × the capacity of one cell
```

### 3S1P

Three cells in series:

```text
Cell ── Cell ── Cell

11.1 V nominal / 12.6 V full
```

### 3S2P

Six cells:

```text
[Cell || Cell] ── [Cell || Cell] ── [Cell || Cell]

11.1 V nominal
Capacity ≈ 2 × the capacity of one cell
```

## Building your own 18650/21700 battery

For a DIY pack, use cells that are:

- the same chemistry and size
- the same model and rating
- from a reputable source
- matched as a set rather than mixing old and new cells

### Recommended build sequence

1. Decide 2S or 3S from the motor voltage.
2. Decide 1P or 2P (or more) from the desired runtime/current capability.
3. Use a **BMS designed for the exact series count** if you are building a rechargeable Li-ion pack.
4. Prefer spot welding with proper nickel strip rather than soldering directly to bare cell ends.
5. Insulate every exposed positive terminal and use cell spacers/fish-paper where appropriate.
6. Add a fuse close to battery positive.
7. Add a master switch that is rated for the expected current.
8. Bring the pack to the correct charger/balance-charger settings before using it on the robot.
9. Measure the pack voltage with a multimeter before connecting the robot.

### BMS selection

The BMS is matched to the number of series cells (2S, 3S, etc.) and must not become the current bottleneck. Choose a continuous-current rating with headroom over the robot's expected battery current. The exact rating depends on the motor choice and duty cycle.

### LiPo option

A ready-made 2S or 3S RC LiPo is often easier for a first build. Use a balance charger intended for the pack chemistry and cell count, and use a low-voltage warning/cutoff appropriate to the battery.

## Power distribution

Recommended architecture:

```text
Battery +
   |
  Fuse
   |
Master switch
   +-----------------------> Motor drivers (VM)
   |
   +-----------------------> Buck/BEC -> 5 V logic/servo/receiver
   |
   +-----------------------> optional voltage monitor

Battery - ------------------> Common GND ----------------> all electronics
```

Keep high-current motor wiring short and suitably thick. Keep signal wires away from motor leads when practical.

## Fuse selection

Do not copy one fuse value into every robot. Select the fuse from:

- expected running current
- acceptable peak current during acceleration
- wire capacity
- driver capacity
- battery capability

For a small prototype, a 5–10 A fuse may be a reasonable starting range for a 2WD robot, and 10–15 A may be a starting range for a small 4WD robot, but **measure your actual system current before finalizing the fuse**.

## Runtime estimate

A rough ideal estimate is:

```text
Runtime (hours) ≈ battery capacity (Ah) / average current (A)
```

Real runtime is lower because of drivetrain losses, regulator losses, acceleration peaks, battery voltage sag and the fact that a Li-ion/LiPo pack should not normally be driven to its theoretical zero-voltage endpoint.
