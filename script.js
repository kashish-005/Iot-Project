import { db, auth, ref, onValue, signInAnonymously } from "./firebase.js";

async function connectFirebase() {
	try {
		await signInAnonymously(auth);
		console.log("Firebase connected successfully!");

		const agriGuardRef = ref(db, "AgriGuard");

		onValue(
			agriGuardRef,
			(snapshot) => {
				const data = snapshot.val();

				if (!data) {
					console.log("No AgriGuard data found in Firebase.");
					return;
				}

				console.log("Latest Firebase data:", data);

				const moisture = Number(data.soil?.moisturePercent);
				const soilStatus = data.soil?.status ?? "UNKNOWN";
				const pumpStatus = String(
					data.irrigation?.pumpStatus ?? "UNKNOWN",
				).toUpperCase();

				// Update soil moisture gauge.
				if (Number.isFinite(moisture)) {
					if (typeof window.updateMoisture === "function") {
						window.updateMoisture(moisture);
					} else {
						document.getElementById("statMoisture").textContent = moisture;
						document.getElementById("gaugeValue").textContent = moisture;
						document
							.getElementById("moistureGauge")
							?.style.setProperty("--moisture", `${moisture}%`);
					}
				}

				// Update soil status.
				const moistureState = document.getElementById("moistureState");
				const moistureMessage = document.getElementById("moistureMessage");
				const moistureCaption = document.getElementById("moistureCaption");

				if (moistureState) {
					moistureState.textContent = soilStatus;
				}

				if (moistureMessage) {
					moistureMessage.textContent =
						"Soil moisture status received from Firebase.";
				}

				if (moistureCaption) {
					moistureCaption.textContent = "Firebase reading";
				}

				// Update pump status.
				if (pumpStatus === "ON" || pumpStatus === "OFF") {
					if (typeof window.setPump === "function") {
						window.setPump(pumpStatus === "ON", false);
					} else {
						document.getElementById("statPump").textContent = pumpStatus;

						document.getElementById("pumpStatusText").textContent =
							`Pump is ${pumpStatus} (Firebase)`;

						document
							.getElementById("pumpStatus")
							?.classList.toggle("running", pumpStatus === "ON");
					}
				} else {
					document.getElementById("statPump").textContent = pumpStatus;
					document.getElementById("pumpStatusText").textContent =
						`Pump status: ${pumpStatus}`;
				}

				console.log("Dashboard updated from Firebase:", {
					moisture,
					soilStatus,
					pumpStatus,
				});
			},
			(error) => {
				console.error("Firebase data listener error:", error);
			},
		);
	} catch (error) {
		console.error("Firebase connection error:", error);
	}
}

connectFirebase();
