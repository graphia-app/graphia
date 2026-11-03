/*
 * Copyright © 2013-2025 Tim Angus
 * Copyright © 2013-2025 Tom Freeman
 *
 * This file is part of Graphia.
 *
 * Graphia is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Graphia is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Graphia.  If not, see <http://www.gnu.org/licenses/>.
 */

// Qt builds a wasm executable as a module factory, so running its .js alone does nothing
const path = require("path");

// Qt's event dispatcher reaches its timer functions through window
globalThis.window = globalThis;

const [testScript, ...testArguments] = process.argv.slice(2);
const createTest = require(path.resolve(testScript));

createTest({
    arguments: testArguments,
    preRun: [(module) => Object.assign(module.ENV, process.env)],
    onExit: (code) => process.exit(code),
    onAbort: (reason) =>
    {
        console.error(`Aborted: ${reason}`);
        process.exit(1);
    }
});
