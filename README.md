
<div align="center">
<img src="https://cdn.discordapp.com/attachments/1112820781991329934/1172370629304008807/image.png?ex=6560123b&is=654d9d3b&hm=79e86b83f0560b5c508a91240cefede0e7bf79f48e7b7382cc82610685b7f2b0&" width="56%">


### **`Linux` cache dropper in `C` without any alloc🎈**

<br>
</div>

<img src="https://cdn.discordapp.com/attachments/1112820781991329934/1172375632332722288/image.png?ex=656016e4&is=654da1e4&hm=3b4efa11a6793114fae16c5a6b12bba8f0cddb88bf684d45d9076055eddad035&" width="50%" align="right">

# Description 📖
A few days ago I noticed that while **playing** a game my `OS` is **caching** all the `RAM` and then frantically **trying** to cram a **new game level** into the *few hundred free migabytes*, at *this point* the **game** is a bit **slow** and it's a bit annoying. So I *wrote* a **small** but very **dumb** `solution` that **just works**.

In **fact**, I'm really *surprised* at *how much* such a **dumb** decision could **fix** the **situation**. I don't **fully** *understand* why this **happens**, so if you **do**, *please open* an ``issue`` and **help** me out.

At this point `ссс` is in a very early stage of development so feel free to open an **issue** with your problems and suggestions.

> **Note** that this is only **compatible** with the `linux` kernel at this time.

<br>

# Installation ☁️
From source
```
git clone https://github.com/ssleert/ccc.git
cd ./ccc
sudo make install
```

# Configuration ⚙️
```fish
/usr/local/etc/ccc.conf
```

<details>
<summary>config file example</summary>

```ini
[Files]
  Lock = /tmp/ccc.lock

[Log]
  Silent = false

[Options]
  Sync = true

[Levels] # percents
  First  = 15
  Second = 10
  Third  = 5

[Timeouts] # seconds
  Check = 10

[Error]
  MaxAmount = 10

```

</details>

# Contribute
Before contributing, please run
```fish
make format
```

<br>
<div align="center">

### made with 🫀 by `sfome`

</div>
