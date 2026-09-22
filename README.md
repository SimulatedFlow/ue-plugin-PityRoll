# PityRoll

Bad-luck protection, soft pity and drop guarantees for Unreal Engine 5.8.

A drop chance of five percent means somebody goes eighty tries without it, and that player does not
think *variance* - that player thinks the game is broken. PityRoll puts a floor under bad luck. It
counts consecutive misses **per entry**, so picking up a common cannot reset the counter that was
building towards the rare one.

* Soft pity: the chance climbs with every miss, from an attempt you choose
* Hard pity: the guarantee lands on **exactly** the attempt you named, not one early and not one late
* The effective chance is a pure function, so the number your UI shows is the number the roll used
* A natural success inside the pity window consumes the counter exactly once
* The random value is handed in, so a server, a client prediction and a replay agree
* Every rule is a pure function the component and the tests both call

Documentation: https://wiki.teufel-engineering.com/en/PityRoll/documentation
Support: teufelsilvan@gmail.com

Unreal Engine 5.8 - Win64 - one runtime C++ module - no third-party code - full source included.

<!-- SF-STORE-BLOCK:BEGIN -->
## 🛒 Source-available — see before you buy

This repository contains the **full source** of a commercial Unreal Engine plugin. It is **source-available, not open source**: read it, evaluate it, then buy a license to use it. See **the Fab Content License Agreement / Unreal Engine EULA (purchase required)**.

**Get it / Buy:**
- Fab store — all our UE5 plugins: https://www.fab.com/sellers/Silvan%20Teufel

_This plugin does not have its own Fab listing yet — the store link above is where everything we currently sell lives._

### 📬 **Free UE5 Snippet-Pack**

10 ready-to-use C++/Blueprint building blocks (subsystems, versioned saves, async nodes, editor tooling) — MIT licensed. Get it by joining the newsletter — plus a heads-up when something new ships. Double opt-in, unsubscribe in one click, no address sharing.

👉 **[Get the free pack](https://silvan.teufel-engineering.com/newsletter/plugins/?q=gh)**

_© 2026 Silvan Teufel. All rights reserved._
<!-- SF-STORE-BLOCK:END -->
