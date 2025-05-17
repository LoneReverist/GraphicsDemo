// Input.ixx

export module Input;

import <mutex>;
import <unordered_set>;

export class Input
{
public:
	enum class Key : int
	{
		Right   = 262, // GLFW_KEY_RIGHT
		Left    = 263, // GLFW_KEY_LEFT
		Down    = 264, // GLFW_KEY_DOWN
		Up      = 265  // GLFW_KEY_UP
	};

	void SetKey(int key, bool pressed)
	{
		std::lock_guard lock(m_key_state_mutex);
		if (pressed)
			m_key_state.insert(key);
		else
			m_key_state.erase(key);
	}

	bool KeyIsPressed(int key) const
	{
		std::lock_guard lock(m_key_state_mutex);
		return m_key_state.contains(key);
	}

	bool KeyIsPressed(Key key) const
	{
		return KeyIsPressed(static_cast<int>(key));
	}

private:
	mutable std::mutex m_key_state_mutex;
	std::unordered_set<int> m_key_state;
};
