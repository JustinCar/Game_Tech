#pragma once
#include "../../Common/Vector3.h"
#include "../../Common/Quaternion.h"
#include "../../Common/Vector4.h"

namespace NCL {
	using namespace Maths;
	namespace CSC8503 {
		class GameObject;
		class NetworkState	{
		public:
			NetworkState();
			virtual ~NetworkState();

			Vector3		position;
			Vector4		colour;
			Quaternion	orientation;
			int			stateID;
		};
	}
}

