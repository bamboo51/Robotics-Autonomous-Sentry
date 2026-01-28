# Robotics-Autonomous-Sentry
This is a source code for robot made in KOSEN's robotic class. This project implements as autonomous sentry gun using mDrawbot Kit (Makeblock Orion board). The system uses an ultrasonic sensor to detect intruders, a stepper motot to scan a desginated area, and servo motors to aim and "fire". The operation is managed by state machine (`STARTUP`, `SCANNING`, '`TRACKING`, `FIRING`). The last source code we used in this project is <a href="./last_fix.ino">last_fix.ino</a>\
日本語の説明は下にあります。

## How it Works
The turret's behavior is controlled by a state machine with four distinct states:

1. STARTUP
- Purpose: Initializes the system and indicates readiness.
- Behavior: On power-on, the buzzer plays a short startup melody. Once the melody finishes, the system transitions to the SCANNING state.
2. SCANNING
- Purpose: To search for potential targets within its field of view.
- Behavior:
The stepper motor moves the turret horizontally, panning back and forth across a 165-degree arc (defined by SIZE and step angles).
The ultrasonic sensor measures the distance every 100ms.
If a target is detected closer than TRACKED_DISTANCE (35mm), the state machine transitions to TRACKING.
A buzzer beeps every second to indicate the system is active and searching.
3. TRACKING
- Purpose: To keep the turret aimed at a detected target.
- Behavior:
The stepper motor stops.
The vertical aiming servo (servo 1) adjusts its angle based on the target's distance, calculating the angle using atan2 to point down at the object.
The buzzer emits warning beeps. The frequency of the beeps increases as the target gets closer.
If the target moves away (distance > TRACKED_DISTANCE), the system reverts to SCANNING.
If the target gets too close (distance < FIRED_DISTANCE, 15mm), the state machine transitions to FIRING.
4. FIRING
- Purpose: To simulate firing at the target.
- Behavior:
A short sequence of servo movements is triggered to simulate a firing action (servo 2).
After firing, the system waits for a "reload" period (5 seconds).
Finally, the turret returns to the default position and transitions back to the SCANNING state.

---

# ロボティクス - 自律型哨戒ロボット (テューポーン)

これは高専の知能ロボティクスで制作したロボットのソースコードです。このプロジェクトは、mDrawbotキット（Makeblock Orionボード）を使用した自律型セントリーガンを実装しています。システムは、侵入者を検知するための超音波センサー、指定エリアをスキャンするステッピングモーター、そして照準を合わせて「発砲」するためのサーボモーターを使用します。一連の動作はステートマシン（`STARTUP`, `SCANNING`, `TRACKING`, `FIRING`）によって管理されています。このプロジェクトで最終的に使用したソースコードは<a href="./last_fix.ino">last_fix.ino</a>です。

## 動作原理

タレットの動作は、4つの異なる状態を持つステートマシンによって制御されます。

### 1. `STARTUP` (起動)
*   **目的:** システムを初期化し、準備が完了したことを示します。
*   **動作:** 電源投入時、ブザーが短い起動メロディを再生します。メロディが終了すると、システムは`SCANNING`状態に移行します。

### 2. `SCANNING` (探索)
*   **目的:** 視界内に潜在的なターゲットを探索します。
*   **動作:**
    *   ステッピングモーターがタレットを水平方向に動かし、165度の範囲を往復します（この範囲は`SIZE`とステップ角度によって定義されます）。
    *   超音波センサーが100ミリ秒ごとに距離を測定します。
    *   もし`TRACKED_DISTANCE`（35mm）より近くでターゲットが検出されると、ステートマシンは`TRACKING`状態に移行します。
    *   システムが稼働中で探索中であることを示すため、ブザーが1秒ごとに鳴ります。

### 3. `TRACKING` (追跡)
*   **目的:** 検出したターゲットにタレットを照準し続けます。
*   **動作:**
    *   ステッピングモーターは停止します。
    *   垂直方向の照準用サーボモーター（サーボ1）は、ターゲットとの距離に基づいて角度を調整します。この角度は`atan2`を使って物体を指し下げるように計算されます。
    *   ブザーは警告音を発します。ターゲットが近づくほど、警告音の頻度が高くなります。
    *   ターゲットが離れた場合（距離 > `TRACKED_DISTANCE`）、システムは`SCANNING`状態に戻ります。
    *   ターゲットが非常に近くに来た場合（距離 < `FIRED_DISTANCE`, 15mm）、ステートマシンは`FIRING`状態に移行します。

### 4. `FIRING` (発砲)
*   **目的:** ターゲットへの「発砲」をシミュレートします。
*   **動作:**
    *   「発砲」アクションをシミュレートするため、一連の短いサーボモーターの動作（サーボ2）がトリガーされます。
    *   発砲後、システムは「リロード」期間（5秒間）待機します。
    *   最後に、タレットはデフォルトの位置に戻り、`SCANNING`状態に移行します。
