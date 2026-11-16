import { initializeApp } from "https://www.gstatic.com/firebasejs/13.3.0/firebase-app.js";
import {
	getDatabase,
	ref,
	onValue,
} from "https://www.gstatic.com/firebasejs/13.3.0/firebase-database.js";
import {
	getAuth,
	signInAnonymously,
} from "https://www.gstatic.com/firebasejs/13.3.0/firebase-auth.js";

const firebaseConfig = {
	apiKey: "AIzaSyAypfKPpJtdQroXwoy4raQj9cAU1UZH8sk",
	authDomain: "agriguard-a8862.firebaseapp.com",
	databaseURL: "https://agriguard-a8862-default-rtdb.firebaseio.com",
	projectId: "agriguard-a8862",
	storageBucket: "agriguard-a8862.firebasestorage.app",
	messagingSenderId: "1033302183226",
	appId: "1:1033302183226:web:a96140562aff732a119403",
};

const app = initializeApp(firebaseConfig);

export const db = getDatabase(app);
export const auth = getAuth(app);
export { ref, onValue, signInAnonymously };
