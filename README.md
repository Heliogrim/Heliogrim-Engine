<h1 align="center">
    Ember Engine
</h1>

<p align="center">
    <a href="https://git.neogc.net/ember/engine/-/releases/ember-v0.0.1">
        <!-- <img src="https://img.shields.io/badge/Version-0.0.1-blue.svg?style=flat" alt="Ember Version"> -->
        <img alt="GitLab Release (by release name)" src="https://img.shields.io/gitlab/v/release/ember/engine?display_name=release&gitlab_url=https%3A%2F%2Fgit.neogc.net&include_prereleases&sort=semver">
    </a>
    <a href="https://git.neogc.net/ember/engine/-/blob/master/LICENSE">
        <img src="https://img.shields.io/badge/License-Private-red.svg?style=flat" alt="Ember License">
    </a>
    <a href="https://git.neogc.net/ember/engine/-/pipelines">
        <img alt="Gitlab pipeline status (self-hosted)" src="https://img.shields.io/gitlab/pipeline-status/ember/engine?branch=master&gitlab_url=https%3A%2F%2Fgit.neogc.net&style=flat">
    </a>
    <a href="">
        <img src="https://img.shields.io/badge/Platform-Windows,%20Linux-inactive.svg?style=flat" alt="Ember Platform">
    </a>
</p>

**Ember** is a game engine structured, developed and maintained by Julius.

The project is currently closed-source and under private license. For more information about using and contributing with **Ember** take a look at the [license](#License).

The engine is **built modularly** to sustain maintainability and provide better possibilities to improve, replace or erase any submodule. Based on the splitted definition of requirements for any subsystem, there are some core modules like the **Session Module**, **Proxy Module** and **Scheduler Module** which are required to bind and composite every functionallity.

## Core features

- Assets
- Common
- ECS
- Environment
- Event
- Graphics
- Network
- Physics
- Proxy
- Resource
- Scene
- Scheduler
- Session
- Sounds
- Utilities

### Additional features

- Maybe Shipable<br>
***and much more...***

## Getting started

### Installation and Dependency

Ember requires at least C++20 standart. See the following pages for help and information on how to get started with Ember:

- [Installation Guide](https://git.neogc.net/ember/engine/-/tree/master/docs/install.md)

### Control Flow

Using **Ember** will enforce you to use a customized control flow. The following instructions and constraints are required based on the scheduling and asynchronously resource handling of the whole game engine.

#### Callbacks

If using a callback function registered with a timed / managed execution context you must guarantee not blocking the current execution context.

```c++
struct CustomComponent {

    void update() {
        ...; // Not allowed to block
    }
}

RegisterComponent<CustomComponent>();
```

#### Threading

Using **Ember** you shall not use any external threading or blocking execution model. For any application case computing asynchronously please use the inbuilt Tasking.

#### Tasking

Within **Ember** executing code asynchronously is one key design function of the scheduling system. To maintain the maximum possible usage of available resources, the scheduler will reorder and executes tasks at will, if not constrained by any barrier.

Take note that every execution context called from **Ember** is not an actual thread, but a fiber executed by a pooled thread. Due to this circumstance the pushlished interface will give you some functions to interact with the control flow and execution context.

---

![delay](https://img.shields.io/badge/delay-grey.svg?style=flat)
Will execute a given task with given delay at least.

![execute](https://img.shields.io/badge/execute-grey.svg?style=flat)
Will execute a given task. For more control use an task aggregate.

![await](https://img.shields.io/badge/await-darkgreen.svg?style=flat)
Will suspend the current execution context until the given resource is available / signaled.

![sleep](https://img.shields.io/badge/sleep-blue.svg?style=flat)
Will suspend the current execution context at least for the given time in milliseconds.

![yield](https://img.shields.io/badge/yield-blue.svg?style=flat)
Is a function to suspend the current execution context and continue later.

![desync](https://img.shields.io/badge/desync-darkred.svg?style=flat)
Will erase guarantees related to the current execution context if possible. This might fail if the current context is marked with a control token, to prevent unexpected execution within callback functions.

![sync](https://img.shields.io/badge/sync-darkred.svg?style=flat)
Will enforce guarantees related to the current execution context if possible. This might fail if the current context is marked with a control token, to prevent unexcepted execution within callback functions.

---

Those functions may satisfy any needs of synchronization for timed exection and resource handling. Anyways, there are helper functions to directly access meta data of the current thread, but due to the execution via fiber, the present thread might change.

There is an example of working with a task detaching and re-attaching to the system.

```c++
void unrelated_work() {
    ...;
}

void callback() {
    ...; // Enforced scheduled context

    desync();

    unrelated_work(); // Free scheduled context

    sync(...);

    ...; // Enforced scheduled context
}
```

An common problem while executing with pooled threads is blocking for io operations or resource related operations. This might releaf using control flow helpers.

```c++
void create_entity(...) {

    auto ae = CreateEntity();

    await(ae);

    ae.record<StaticMeshComponent>();
    ...
}

void play_sound(const asset_guid& guid_) {

    auto sound = AssetByGuid<SoundAsset>(guid_);

    await(sound.load());

    sound.play();
    ...
}
```

Another function you should care about is to execute tasks at the same execution stage. This will enable you to execute computational tasks which may block triggered by a timed context.

```c++
struct CustomComponent {

    void compute() {
        ...; // Any computational task which may block

        await(...) / yield / sleep(...); // Allowed to block
    }

    void update() {
        ...; // Not allowed to block, so schedule task which can

        // Build a task which should execute at the same stage
        Task task {
            [this]() {
                compute();
            }
        };

        task.dstStage(...);

        // Schedule the task to execute
        execute(task);
    }
}

RegisterComponent<CustomComponent>();
```

---

Related to the asynchronous computation might be a synchronization for given tasks afar from awaitable signals. Those enforced synchronizations are called stage barriers. The stages are related to a fixed order each containing a barrier with a certain guarantee. The scheduling can only move forward if every guarantee of the current barrier was fulfilled.

Task enqueued to the scheduler may or may not carry two stages. The first stage named `source stage` is the earliest execution stage at which the given task may be executed. The second stage named `destination stage` is the latest execution stage at which the task may be executed.

Based on the `weak` or `strong` guarantee of the given stage barriers, the scheduler will handle the execution guarantee differently.

## Contributing

### Get in touch

<table>
  <tr>
    <td><a href="mailto:julius.richter@gmx.de">Email</a></td>
    <td>Contact me via Email with any request regarding <b>Ember</b></td>
  </tr>
</table>

## License

Ember is licensed under Private License ([Private](https://git.neogc.net/ember/engine/-/blob/master/LICENSE)).