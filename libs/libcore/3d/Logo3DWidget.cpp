#ifdef LOLBOB
#include "Logo3DWidget.hpp"

#include <QGuiApplication>

#include <QPropertyAnimation>




class OrbitTransformController : public QObject
{
		Q_OBJECT
		Q_PROPERTY(Qt3DCore::QTransform* target READ target WRITE setTarget NOTIFY targetChanged)
		Q_PROPERTY(float radius READ radius WRITE setRadius NOTIFY radiusChanged)
		Q_PROPERTY(float angle READ angle WRITE setAngle NOTIFY angleChanged)

	public:
		OrbitTransformController(QObject *parent = nullptr)
			: QObject(parent)
			, m_target(Q_NULLPTR)
			, m_matrix()
			, m_radius(1.0f)
			, m_angle(0.0f)
		{
		}

		void setTarget(Qt3DCore::QTransform *target)
		{
			if (m_target != target) {
				m_target = target;
				emit targetChanged();
			}
		}

		Qt3DCore::QTransform *target() const { return m_target; }

		void setRadius(float radius)
		{
			if (!qFuzzyCompare(radius, m_radius)) {
				m_radius = radius;
				updateMatrix();
				emit radiusChanged();
			}
		}

		float radius() const { return m_radius; }

		void setAngle(float angle)
		{
			if (!qFuzzyCompare(angle, m_angle)) {
				m_angle = angle;
				updateMatrix();
				emit angleChanged();
			}
		}

		float angle() const { return m_angle; }

	signals:
		void targetChanged();
		void radiusChanged();
		void angleChanged();

	protected:
		void updateMatrix()
		{
			m_matrix.setToIdentity();
			m_matrix.rotate(m_angle, QVector3D(0.0f, 1.0f, 0.0f));
			m_matrix.translate(m_radius, 0.0f, 0.0f);
			m_target->setMatrix(m_matrix);
		}

	private:
		Qt3DCore::QTransform *m_target;
		QMatrix4x4 m_matrix;
		float m_radius;
		float m_angle;
};

Logo3DWidget::Logo3DWidget(QObject *parent) : QObject(parent)
{

	Qt3DRender::QWindow view;
	Qt3DInput::QInputAspect *input = new Qt3DInput::QInputAspect;
	view.registerAspect(input);

	// Root entity
	Qt3DCore::QEntity *rootEntity = new Qt3DCore::QEntity();

	// Camera
	Qt3DCore::QCamera *cameraEntity = view.defaultCamera();

	cameraEntity->lens()->setPerspectiveProjection(45.0f, 16.0f/9.0f, 0.1f, 1000.0f);
	cameraEntity->setPosition(QVector3D(0, 0, -40.0f));
	cameraEntity->setUpVector(QVector3D(0, 1, 0));
	cameraEntity->setViewCenter(QVector3D(0, 0, 0));
	input->setCamera(cameraEntity);

	// Material
	Qt3DRender::QMaterial *material = new Qt3DRender::QPhongMaterial(rootEntity);

	// Torus
	Qt3DCore::QEntity *torusEntity = new Qt3DCore::QEntity(rootEntity);
	Qt3DRender::QTorusMesh *torusMesh = new Qt3DRender::QTorusMesh;
	torusMesh->setRadius(5);
	torusMesh->setMinorRadius(1);
	torusMesh->setRings(100);
	torusMesh->setSlices(20);

	Qt3DCore::QTransform *torusTransform = new Qt3DCore::QTransform;
	torusTransform->setScale3D(QVector3D(1.5, 1, 0.5));
	torusTransform->setRotation(QQuaternion::fromAxisAndAngle(QVector3D(1, 0, 0), 45.0f));

	torusEntity->addComponent(torusMesh);
	torusEntity->addComponent(torusTransform);
	torusEntity->addComponent(material);

	// Sphere
	Qt3DCore::QEntity *sphereEntity = new Qt3DCore::QEntity(rootEntity);
	Qt3DRender::QSphereMesh *sphereMesh = new Qt3DRender::QSphereMesh;
	sphereMesh->setRadius(3);

	Qt3DCore::QTransform *sphereTransform = new Qt3DCore::QTransform;
	OrbitTransformController *controller = new OrbitTransformController(sphereTransform);
	controller->setTarget(sphereTransform);
	controller->setRadius(20.0f);

	QPropertyAnimation *sphereRotateTransformAnimation = new QPropertyAnimation(sphereTransform);
	sphereRotateTransformAnimation->setTargetObject(controller);
	sphereRotateTransformAnimation->setPropertyName("angle");
	sphereRotateTransformAnimation->setStartValue(QVariant::fromValue(0));
	sphereRotateTransformAnimation->setEndValue(QVariant::fromValue(360));
	sphereRotateTransformAnimation->setDuration(10000);
	sphereRotateTransformAnimation->setLoopCount(-1);
	sphereRotateTransformAnimation->start();

	sphereEntity->addComponent(sphereMesh);
	sphereEntity->addComponent(sphereTransform);
	sphereEntity->addComponent(material);

	view.setRootEntity(rootEntity);
	view.show();


}
#endif
