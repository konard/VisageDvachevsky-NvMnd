# Спецификация ядра runtime

Этот документ определяет API и интерфейсы ядра runtime для движка NovelMind.

## Слой абстракции платформы

### Интерфейс окна

```cpp
namespace nm::platform
{

struct WindowConfig
{
    std::string title = "NovelMind";
    int width = 1280;
    int height = 720;
    bool fullscreen = false;
    bool resizable = true;
    bool vsync = true;
};

class IWindow
{
public:
    virtual ~IWindow() = default;

    virtual bool create(const WindowConfig& config) = 0;
    virtual void destroy() = 0;

    virtual void setTitle(const std::string& title) = 0;
    virtual void setSize(int width, int height) = 0;
    virtual void setFullscreen(bool fullscreen) = 0;

    [[nodiscard]] virtual int getWidth() const = 0;
    [[nodiscard]] virtual int getHeight() const = 0;
    [[nodiscard]] virtual bool isFullscreen() const = 0;
    [[nodiscard]] virtual bool shouldClose() const = 0;

    virtual void pollEvents() = 0;
    virtual void swapBuffers() = 0;

    [[nodiscard]] virtual void* getNativeHandle() const = 0;
};

} // namespace nm::platform
```

### Интерфейс таймера

```cpp
namespace nm::platform
{

class ITimer
{
public:
    virtual ~ITimer() = default;

    virtual void reset() = 0;

    [[nodiscard]] virtual double getElapsedSeconds() const = 0;
    [[nodiscard]] virtual double getElapsedMilliseconds() const = 0;
    [[nodiscard]] virtual uint64_t getElapsedMicroseconds() const = 0;

    [[nodiscard]] virtual double getDeltaTime() const = 0;
    virtual void tick() = 0;
};

} // namespace nm::platform
```

### Интерфейс файловой системы

```cpp
namespace nm::platform
{

class IFileSystem
{
public:
    virtual ~IFileSystem() = default;

    [[nodiscard]] virtual Result<std::vector<uint8_t>> readFile(
        const std::string& path) const = 0;

    [[nodiscard]] virtual Result<void> writeFile(
        const std::string& path,
        const std::vector<uint8_t>& data) const = 0;

    [[nodiscard]] virtual bool exists(const std::string& path) const = 0;
    [[nodiscard]] virtual bool isFile(const std::string& path) const = 0;
    [[nodiscard]] virtual bool isDirectory(const std::string& path) const = 0;

    [[nodiscard]] virtual std::string getExecutablePath() const = 0;
    [[nodiscard]] virtual std::string getUserDataPath() const = 0;
};

} // namespace nm::platform
```

## Виртуальная файловая система

### Интерфейс VFS

```cpp
namespace nm::vfs
{

enum class ResourceType
{
    Unknown,
    Texture,
    Audio,
    Font,
    Script,
    Scene,
    Localization,
    Data
};

struct ResourceInfo
{
    std::string id;
    ResourceType type;
    size_t size;
    uint32_t checksum;
};

class IVirtualFileSystem
{
public:
    virtual ~IVirtualFileSystem() = default;

    virtual Result<void> mount(const std::string& packPath) = 0;
    virtual void unmount(const std::string& packPath) = 0;
    virtual void unmountAll() = 0;

    [[nodiscard]] virtual Result<std::vector<uint8_t>> readFile(
        const std::string& resourceId) const = 0;

    [[nodiscard]] virtual bool exists(const std::string& resourceId) const = 0;
    [[nodiscard]] virtual std::optional<ResourceInfo> getInfo(
        const std::string& resourceId) const = 0;

    [[nodiscard]] virtual std::vector<std::string> listResources(
        ResourceType type = ResourceType::Unknown) const = 0;
};

} // namespace nm::vfs
```

### Дескриптор ресурса

```cpp
namespace nm::vfs
{

template<typename T>
class ResourceHandle
{
public:
    ResourceHandle() = default;
    explicit ResourceHandle(std::shared_ptr<T> resource);

    [[nodiscard]] bool isValid() const;
    [[nodiscard]] T* get() const;
    [[nodiscard]] T* operator->() const;
    [[nodiscard]] T& operator*() const;

    void reset();

private:
    std::shared_ptr<T> m_resource;
};

} // namespace nm::vfs
```

## Менеджер ресурсов

### Интерфейс менеджера ресурсов

```cpp
namespace nm::resource
{

class ResourceManager
{
public:
    explicit ResourceManager(nm::vfs::IVirtualFileSystem& vfs);
    ~ResourceManager();

    Result<void> initialize();
    void shutdown();

    // Загрузка текстур
    [[nodiscard]] Result<TextureHandle> loadTexture(const std::string& id);
    void unloadTexture(const std::string& id);

    // Загрузка шрифтов
    [[nodiscard]] Result<FontHandle> loadFont(const std::string& id, int size);
    void unloadFont(const std::string& id);

    // Загрузка аудио
    [[nodiscard]] Result<SoundHandle> loadSound(const std::string& id);
    [[nodiscard]] Result<MusicHandle> loadMusic(const std::string& id);
    void unloadSound(const std::string& id);
    void unloadMusic(const std::string& id);

    // Загрузка скриптов
    [[nodiscard]] Result<ScriptHandle> loadScript(const std::string& id);
    void unloadScript(const std::string& id);

    // Управление кешем
    void clearCache();
    void preload(const std::vector<std::string>& resourceIds);

    [[nodiscard]] size_t getCacheSize() const;
    [[nodiscard]] size_t getResourceCount() const;

private:
    nm::vfs::IVirtualFileSystem& m_vfs;
    // Внутренние кеши...
};

} // namespace nm::resource
```

## Рендерер

### Интерфейс рендерера

```cpp
namespace nm::renderer
{

struct Color
{
    uint8_t r, g, b, a;

    static const Color White;
    static const Color Black;
    static const Color Transparent;
};

struct Rect
{
    float x, y, width, height;
};

struct Transform2D
{
    float x = 0.0f;
    float y = 0.0f;
    float scaleX = 1.0f;
    float scaleY = 1.0f;
    float rotation = 0.0f;
    float anchorX = 0.0f;
    float anchorY = 0.0f;
};

enum class BlendMode
{
    None,
    Alpha,
    Additive,
    Multiply
};

class IRenderer
{
public:
    virtual ~IRenderer() = default;

    virtual Result<void> initialize(nm::platform::IWindow& window) = 0;
    virtual void shutdown() = 0;

    virtual void beginFrame() = 0;
    virtual void endFrame() = 0;

    virtual void clear(const Color& color) = 0;

    virtual void setBlendMode(BlendMode mode) = 0;

    // Рендеринг спрайтов
    virtual void drawSprite(
        const Texture& texture,
        const Transform2D& transform,
        const Color& tint = Color::White) = 0;

    virtual void drawSprite(
        const Texture& texture,
        const Rect& sourceRect,
        const Transform2D& transform,
        const Color& tint = Color::White) = 0;

    // Рендеринг текста
    virtual void drawText(
        const Font& font,
        const std::string& text,
        float x, float y,
        const Color& color = Color::White) = 0;

    // Рендеринг примитивов
    virtual void drawRect(const Rect& rect, const Color& color) = 0;
    virtual void fillRect(const Rect& rect, const Color& color) = 0;

    // Эффекты экрана
    virtual void setFade(float alpha, const Color& color = Color::Black) = 0;

    [[nodiscard]] virtual int getWidth() const = 0;
    [[nodiscard]] virtual int getHeight() const = 0;
};

} // namespace nm::renderer
```

### Текстура

```cpp
namespace nm::renderer
{

class Texture
{
public:
    Texture() = default;
    ~Texture();

    Texture(const Texture&) = delete;
    Texture& operator=(const Texture&) = delete;
    Texture(Texture&& other) noexcept;
    Texture& operator=(Texture&& other) noexcept;

    Result<void> loadFromMemory(const std::vector<uint8_t>& data);
    void destroy();

    [[nodiscard]] bool isValid() const;
    [[nodiscard]] int getWidth() const;
    [[nodiscard]] int getHeight() const;
    [[nodiscard]] void* getNativeHandle() const;

private:
    void* m_handle = nullptr;
    int m_width = 0;
    int m_height = 0;
};

} // namespace nm::renderer
```

## Движок скриптов

### Типы скриптов

```cpp
namespace nm::scripting
{

enum class OpCode : uint8_t
{
    // Управление потоком
    NOP,
    HALT,
    JUMP,
    JUMP_IF,
    JUMP_IF_NOT,
    CALL,
    RETURN,

    // Операции со стеком
    PUSH_INT,
    PUSH_FLOAT,
    PUSH_STRING,
    PUSH_BOOL,
    PUSH_NULL,
    POP,
    DUP,

    // Переменные
    LOAD_VAR,
    STORE_VAR,
    LOAD_GLOBAL,
    STORE_GLOBAL,

    // Арифметика
    ADD,
    SUB,
    MUL,
    DIV,
    MOD,
    NEG,

    // Сравнение
    EQ,
    NE,
    LT,
    LE,
    GT,
    GE,

    // Логика
    AND,
    OR,
    NOT,

    // Команды визуальной новеллы
    SHOW_BACKGROUND,
    SHOW_CHARACTER,
    HIDE_CHARACTER,
    SAY,
    CHOICE,
    SET_FLAG,
    CHECK_FLAG,
    PLAY_SOUND,
    PLAY_MUSIC,
    STOP_MUSIC,
    WAIT,
    TRANSITION,
    GOTO_SCENE
};

struct Instruction
{
    OpCode opcode;
    uint32_t operand;
};

} // namespace nm::scripting
```

### Интерпретатор скриптов

```cpp
namespace nm::scripting
{

class ScriptInterpreter
{
public:
    ScriptInterpreter();
    ~ScriptInterpreter();

    Result<void> loadScript(const std::vector<uint8_t>& bytecode);
    void reset();

    void setCallback(OpCode op, std::function<void(uint32_t)> callback);

    bool step();
    void run();
    void pause();
    void resume();

    [[nodiscard]] bool isRunning() const;
    [[nodiscard]] bool isPaused() const;
    [[nodiscard]] bool isWaiting() const;

    void setVariable(const std::string& name, int value);
    void setVariable(const std::string& name, float value);
    void setVariable(const std::string& name, const std::string& value);
    void setVariable(const std::string& name, bool value);

    [[nodiscard]] std::optional<int> getIntVariable(const std::string& name) const;
    [[nodiscard]] std::optional<float> getFloatVariable(const std::string& name) const;
    [[nodiscard]] std::optional<std::string> getStringVariable(const std::string& name) const;
    [[nodiscard]] std::optional<bool> getBoolVariable(const std::string& name) const;

private:
    // Детали реализации
};

} // namespace nm::scripting
```

## Система сцен

### Менеджер сцен

```cpp
namespace nm::scene
{

enum class LayerType
{
    Background,
    Characters,
    UI,
    Effects
};

class SceneManager
{
public:
    SceneManager();
    ~SceneManager();

    Result<void> loadScene(const std::string& sceneId);
    void unloadScene();

    void update(float deltaTime);
    void render(nm::renderer::IRenderer& renderer);

    // Управление слоями
    void addToLayer(LayerType layer, std::unique_ptr<SceneObject> object);
    void removeFromLayer(LayerType layer, const std::string& objectId);
    void clearLayer(LayerType layer);

    // Доступ к объектам
    SceneObject* findObject(const std::string& id);

    // Переходы
    void startTransition(TransitionType type, float duration);
    [[nodiscard]] bool isTransitioning() const;

private:
    // Детали реализации
};

} // namespace nm::scene
```

### Объект сцены

```cpp
namespace nm::scene
{

class SceneObject
{
public:
    explicit SceneObject(const std::string& id);
    virtual ~SceneObject() = default;

    [[nodiscard]] const std::string& getId() const;

    void setPosition(float x, float y);
    void setScale(float scaleX, float scaleY);
    void setRotation(float angle);
    void setAlpha(float alpha);
    void setVisible(bool visible);

    [[nodiscard]] const Transform2D& getTransform() const;
    [[nodiscard]] float getAlpha() const;
    [[nodiscard]] bool isVisible() const;

    virtual void update(float deltaTime);
    virtual void render(nm::renderer::IRenderer& renderer) = 0;

protected:
    std::string m_id;
    Transform2D m_transform;
    float m_alpha = 1.0f;
    bool m_visible = true;
};

} // namespace nm::scene
```

## Система ввода

### Менеджер ввода

```cpp
namespace nm::input
{

enum class Key
{
    Unknown,
    Space,
    Enter,
    Escape,
    Up, Down, Left, Right,
    A, B, C, /* ... */ Z,
    Num0, Num1, /* ... */ Num9,
    F1, F2, /* ... */ F12,
    Shift, Ctrl, Alt
};

enum class MouseButton
{
    Left,
    Right,
    Middle
};

class InputManager
{
public:
    InputManager();
    ~InputManager();

    void update();

    // Клавиатура
    [[nodiscard]] bool isKeyDown(Key key) const;
    [[nodiscard]] bool isKeyPressed(Key key) const;
    [[nodiscard]] bool isKeyReleased(Key key) const;

    // Мышь
    [[nodiscard]] bool isMouseButtonDown(MouseButton button) const;
    [[nodiscard]] bool isMouseButtonPressed(MouseButton button) const;
    [[nodiscard]] bool isMouseButtonReleased(MouseButton button) const;
    [[nodiscard]] int getMouseX() const;
    [[nodiscard]] int getMouseY() const;

    // Текстовый ввод
    void startTextInput();
    void stopTextInput();
    [[nodiscard]] const std::string& getTextInput() const;

private:
    // Детали реализации
};

} // namespace nm::input
```

## Аудио система

### Менеджер аудио

```cpp
namespace nm::audio
{

class AudioManager
{
public:
    AudioManager();
    ~AudioManager();

    Result<void> initialize();
    void shutdown();

    // Звуковые эффекты
    void playSound(const Sound& sound, float volume = 1.0f);
    void stopAllSounds();

    // Музыка
    void playMusic(const Music& music, bool loop = true);
    void stopMusic();
    void pauseMusic();
    void resumeMusic();
    void setMusicVolume(float volume);
    [[nodiscard]] bool isMusicPlaying() const;

    // Общая громкость
    void setMasterVolume(float volume);
    [[nodiscard]] float getMasterVolume() const;

private:
    // Детали реализации
};

} // namespace nm::audio
```

## Система сохранений

### Менеджер сохранений

```cpp
namespace nm::save
{

struct SaveData
{
    std::string sceneId;
    std::string nodeId;
    std::map<std::string, int> intVariables;
    std::map<std::string, bool> flags;
    std::map<std::string, std::string> stringVariables;
    uint64_t timestamp;
    uint32_t checksum;
};

class SaveManager
{
public:
    SaveManager(nm::platform::IFileSystem& fs);
    ~SaveManager();

    Result<void> save(int slot, const SaveData& data);
    Result<SaveData> load(int slot);
    Result<void> deleteSave(int slot);

    [[nodiscard]] bool slotExists(int slot) const;
    [[nodiscard]] std::optional<uint64_t> getSlotTimestamp(int slot) const;

    [[nodiscard]] int getMaxSlots() const;

private:
    nm::platform::IFileSystem& m_fs;
    static constexpr int MAX_SLOTS = 100;
};

} // namespace nm::save
```

## Основное приложение

### Класс приложения

```cpp
namespace nm::core
{

struct EngineConfig
{
    platform::WindowConfig window;
    std::string packFile;
    std::string startScene;
    bool debug = false;
};

class Application
{
public:
    Application();
    ~Application();

    Application(const Application&) = delete;
    Application& operator=(const Application&) = delete;

    Result<void> initialize(const EngineConfig& config);
    void shutdown();

    void run();
    void quit();

    [[nodiscard]] platform::IWindow& getWindow();
    [[nodiscard]] renderer::IRenderer& getRenderer();
    [[nodiscard]] resource::ResourceManager& getResources();
    [[nodiscard]] scene::SceneManager& getSceneManager();
    [[nodiscard]] input::InputManager& getInput();
    [[nodiscard]] audio::AudioManager& getAudio();
    [[nodiscard]] save::SaveManager& getSaveManager();

private:
    void mainLoop();
    void processEvents();
    void update(float deltaTime);
    void render();

    bool m_running = false;
    EngineConfig m_config;

    // Подсистемы
    std::unique_ptr<platform::IWindow> m_window;
    std::unique_ptr<platform::ITimer> m_timer;
    std::unique_ptr<platform::IFileSystem> m_fileSystem;
    std::unique_ptr<vfs::IVirtualFileSystem> m_vfs;
    std::unique_ptr<renderer::IRenderer> m_renderer;
    std::unique_ptr<resource::ResourceManager> m_resources;
    std::unique_ptr<scene::SceneManager> m_sceneManager;
    std::unique_ptr<scripting::ScriptInterpreter> m_scripting;
    std::unique_ptr<input::InputManager> m_input;
    std::unique_ptr<audio::AudioManager> m_audio;
    std::unique_ptr<save::SaveManager> m_saveManager;
};

} // namespace nm::core
```

## Система логирования

### Интерфейс логгера

```cpp
namespace nm::core
{

enum class LogLevel
{
    Trace,
    Debug,
    Info,
    Warning,
    Error,
    Fatal
};

class Logger
{
public:
    static Logger& instance();

    void setLevel(LogLevel level);
    void setOutputFile(const std::string& path);

    template<typename... Args>
    void trace(const std::string& format, Args&&... args);

    template<typename... Args>
    void debug(const std::string& format, Args&&... args);

    template<typename... Args>
    void info(const std::string& format, Args&&... args);

    template<typename... Args>
    void warning(const std::string& format, Args&&... args);

    template<typename... Args>
    void error(const std::string& format, Args&&... args);

    template<typename... Args>
    void fatal(const std::string& format, Args&&... args);

private:
    Logger() = default;
    void log(LogLevel level, const std::string& message);
};

// Удобные макросы
#define NM_LOG_TRACE(...) nm::core::Logger::instance().trace(__VA_ARGS__)
#define NM_LOG_DEBUG(...) nm::core::Logger::instance().debug(__VA_ARGS__)
#define NM_LOG_INFO(...)  nm::core::Logger::instance().info(__VA_ARGS__)
#define NM_LOG_WARN(...)  nm::core::Logger::instance().warning(__VA_ARGS__)
#define NM_LOG_ERROR(...) nm::core::Logger::instance().error(__VA_ARGS__)
#define NM_LOG_FATAL(...) nm::core::Logger::instance().fatal(__VA_ARGS__)

} // namespace nm::core
```
