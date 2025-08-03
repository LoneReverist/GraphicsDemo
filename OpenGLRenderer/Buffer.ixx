// Buffer.ixx

module;

export module Buffer;

export class Buffer
{
public:
	Buffer() = default;
	~Buffer();

	Buffer(Buffer && other) noexcept;
	Buffer & operator=(Buffer && other) noexcept;

	Buffer(Buffer const &) = delete;
	Buffer & operator=(Buffer const &) = delete;

	void Create();
	void Destroy();

	unsigned int GetId() const { return m_id; }

private:
	unsigned int m_id = 0;
};
