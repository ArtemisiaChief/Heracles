#include <iostream>
#include <sstream>

#include "graphic_renderer.h"
#include "resource_manager.h"

namespace heracles {

	// ����
	GLFWwindow* graphic_renderer::window_ = nullptr;

	// ��ɫ������
	shader* graphic_renderer::shader_program_ = nullptr;

	// ����
	world* graphic_renderer::the_world_ = nullptr;

	// ���ڴ�С��ʼ����
	GLint graphic_renderer::win_width_ = 1600;
	GLint graphic_renderer::win_height_ = 900;

	// �ӵ�������ͶӰ����
	GLfloat graphic_renderer::zoom_ = 0.0f;
	vec2 graphic_renderer::view_(0.0f, 0.0f);
	mat22 graphic_renderer::projection_(8.0f / win_width_, 0, 0, 8.0f / win_height_);

	text* graphic_renderer::text_ = nullptr;

	// ���Ƹ���
	void graphic_renderer::draw_body(polygon_body& body) {
		resource_manager::get_shader("graphic").use().set_vec2("translation", body.get_world_position());
		resource_manager::get_shader("graphic").set_mat22("rotation", body.get_rotation());

		//resource_manager::get_texture("test1").bind();
		glBindVertexArray(*body.get_id());
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	}

	// ���ƽ���
	void graphic_renderer::draw_joint() {

	}

	// ��������ʾdt
	void graphic_renderer::update_title(const double dt) {
		std::stringstream ss;
		ss << "Heracles - dt: " << std::to_string(dt * 1000).substr(0, 5) << " ms";
		glfwSetWindowTitle(window_, ss.str().c_str());
	}

	// ʱ��ͬ��
	auto graphic_renderer::diff_time() {
		using namespace std::chrono;
		using seconds = duration<double>;
		static auto last_clock = high_resolution_clock::now();		// ÿ�ε���high_resolution_clock::now()
		const auto now = high_resolution_clock::now();				// ��һ��һ����ǰһ�δ�
		const auto dt = duration_cast<seconds>(now - last_clock);	// ��֤tick����һ���ȶ���ʱ����
		update_title(dt.count());									// ��ʾdt
		last_clock = now;
		return dt;
	}

	// �󶨸������Ķ��㣬�´����ĸ���ֻ��Ҫ����һ�θú���
	void graphic_renderer::bind_vertex_array(polygon_body& body) {
		auto vertices = body.get_vertices();

		float tex_coord[] = {
			0.0f, 0.0f,
			0.0f, 1.0f,
			1.0f, 0.0f,
			1.0f, 1.0f
		};

		// ���ö������飬���ö����������VAO���붥�㻺�����VBO��
		const auto vao = body.get_id();
		unsigned int vbo;
		glGenVertexArrays(1, vao);
		glGenBuffers(1, &vbo);

		// ��������
		glBindVertexArray(*vao);

		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 16, nullptr, GL_STATIC_DRAW);

		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), nullptr);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), reinterpret_cast<void*>(8 * sizeof(float)));
		glEnableVertexAttribArray(1);

		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float) * 8, &vertices[0]);
		glBufferSubData(GL_ARRAY_BUFFER, sizeof(float) * 8, sizeof(float) * 8, tex_coord);

		// ��������
		//resource_manager::load_texture("src/resources/container.jpg", "test1");
	
		//glBindBuffer(GL_ARRAY_BUFFER, 0);
		//glBindVertexArray(0);
	}

	// ��Ⱦ - ���� OpenGL ���Ʒ����������
	void graphic_renderer::display() {
		glClear(GL_COLOR_BUFFER_BIT);

		for (auto &body : the_world_->get_bodies())
			draw_body(*std::dynamic_pointer_cast<polygon_body>(body).get());

		text_->render_text("Heracles", -780.0f, -430.0f, 1.0f, 1.0f, 1.0f, 1.0f);

		// glfw˫����+�����¼�
		glfwSwapBuffers(window_);
	}

	// ������ƶ�
	void graphic_renderer::move_camera(const vec2 translation) {
		view_ += translation;
		resource_manager::get_shader("graphic").use().set_vec2("view", view_);
	}

	// ��갴���ص�����
	void graphic_renderer::mouse_callback(GLFWwindow* window, const int button, const int action, const int mods) {
		if (action == GLFW_PRESS) switch (button) {
			//���������ø���
		case GLFW_MOUSE_BUTTON_LEFT: {
			double x, y;
			glfwGetCursorPos(window, &x, &y);
			const auto half_width = win_width_ / 2;
			const auto half_height = win_height_ / 2;
			vec2 pos((x - half_width) / half_width, (-y + half_height) / half_height);
			pos = projection_.inv() * pos + view_;

			// ���紴�����
			auto body = the_world_->create_box(1, 10, 10, pos);
			the_world_->add(body);

			//�󶨸���Ķ�������
			bind_vertex_array(*std::dynamic_pointer_cast<polygon_body>(body).get());

			std::cout << "Create Box (" << body->get_id() << ") : [" << pos.x << " " << pos.y << "]" << std::endl;
		}

		// ����Ҽ���ĳ������ʩ����
		case GLFW_MOUSE_BUTTON_RIGHT: {
			break;
		}
		default: ;
		}
	}

	// �����ֻص�����
	void graphic_renderer::scroll_callback(GLFWwindow* window, const double xoffset, const double yoffset) {
		if (zoom_ >= -10.0f && zoom_ <= 25.0f) {
			zoom_ += yoffset;
			projection_[0].x = 8.0f / win_width_ + zoom_ / (win_width_ * 2.5f);
			projection_[1].y = 8.0f / win_height_ + zoom_ / (win_height_ * 2.5f);
			resource_manager::get_shader("graphic").use().set_mat22("projection", projection_);
		}
		if (zoom_ <= -10.0f)
			zoom_ = -10.0f;
		if (zoom_ >= 25.0f)
			zoom_ = 25.0f;
	}

	// ��������
	void graphic_renderer::process_input() {

		if (glfwGetKey(window_, GLFW_KEY_ESCAPE) == GLFW_PRESS)
			glfwSetWindowShouldClose(window_, true);

		auto camera_speed = 2.0f;
		if (glfwGetKey(window_, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
			camera_speed *= 3;
		if (glfwGetKey(window_, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
			camera_speed /= 3;
		if (glfwGetKey(window_, GLFW_KEY_W) == GLFW_PRESS)
			move_camera(vec2(0, 1 * camera_speed));
		if (glfwGetKey(window_, GLFW_KEY_A) == GLFW_PRESS)
			move_camera(vec2(-1 * camera_speed, 0));
		if (glfwGetKey(window_, GLFW_KEY_S) == GLFW_PRESS)
			move_camera(vec2(0, -1 * camera_speed));
		if (glfwGetKey(window_, GLFW_KEY_D) == GLFW_PRESS)
			move_camera(vec2(1 * camera_speed, 0));
	}

	// ���ڴ�С����ʱ���ӿڣ�Viewport�����е����Ļص�����
	void graphic_renderer::framebuffer_size_callback(GLFWwindow* window, const int width, const int height) {
		glViewport(0, 0, width, height);
		win_width_ = width;
		win_height_ = height;
		projection_[0].x = 8.0f / win_width_ + zoom_ / (win_width_ * 2.5f);
		projection_[1].y = 8.0f / win_height_ + zoom_ / (win_height_ * 2.5f);
		resource_manager::get_shader("graphic").use().set_mat22("projection", projection_);
	}

	// �����������в���
	static std::atomic<bool> should_stop{ false };
	void graphic_renderer::heracles_run() {
		using namespace std::chrono_literals;
		while (!should_stop) {
			std::this_thread::sleep_for(10ms);
			auto dt = diff_time().count();

			// ��������һ��������Step��������
			the_world_->step(dt);
		}
	}

	// ��ʼ��
	int graphic_renderer::init() {
		// glfw��ʼ��
		glfwInit();
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

		// glfw���ڴ���
		window_ = glfwCreateWindow(win_width_, win_height_, "Heracles", nullptr, nullptr);
		if (window_ == nullptr) {
			glfwTerminate();
			return -1;
		}
		glfwMakeContextCurrent(window_);
		glfwSetFramebufferSizeCallback(window_, framebuffer_size_callback);

		// �������ص�����
		glfwSetMouseButtonCallback(window_, mouse_callback);
		glfwSetScrollCallback(window_, scroll_callback);

		// glad����OpenGL����ָ��
		if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress))) {
			return -2;
		}

		// �������
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		// ��������
		text_ = new text(win_width_, win_height_);
		text_->load("src/resources/consola.ttf", 48);

		// ���첢ʹ�õ���ɫ����Ƭ����ɫ��
		resource_manager::load_shader("src/graphics_engine/shader/graphic.v", "src/graphics_engine/shader/graphic.f", "graphic");
		resource_manager::get_shader("graphic").use().set_vec2("view", view_);
		resource_manager::get_shader("graphic").set_mat22("projection", projection_);

		// �߿�ģʽ
		//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

		the_world_ = new world({ 0.0f, -9.8f });

		return 0;
	}

	// ��Ⱦ��ѭ��
	void graphic_renderer::loop() {
		std::thread heracles_thread(heracles_run);

		while (!glfwWindowShouldClose(window_)) {
			process_input();		// ��������
			display();				// ��ʾͼ��
			glfwPollEvents();		// �����¼�
		}

		glfwTerminate();
		should_stop = true;
		heracles_thread.join();	// ��ֹreturnʱ���������̻߳�û������
	}
}