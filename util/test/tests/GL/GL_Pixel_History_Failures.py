import renderdoc as rd
import rdtest
from rdtest.logging import TestFailureException
from typing import List

class GL_Pixel_History_Failures(rdtest.TestCase):
    demos_test_name = 'GL_Pixel_History_Failures'
    demos_frame_cap = 10

    def check_capture(self):
        action = self.get_first_action()
        i = 0

        while True:
            action: rd.ActionDescription = self.find_action('glDraw', action.eventId+1)

            if action is None:
                break

            rdtest.log.success(f'Test {action.eventId} started.')
            self.controller.SetFrameEvent(action.eventId, True)

            pipe: rd.PipeState = self.controller.GetPipelineState()
            rt: rd.BoundResource = pipe.GetOutputTargets()[0]
            tex: rd.ResourceId = rt.resourceId
            sub = rd.Subresource()

            x, y = 190, 149
            modifs: List[rd.PixelModification] = [x for x in self.controller.PixelHistory(tex, x, y, sub, rt.typeCast) if x.eventId == action.eventId]
            self.check(len(modifs) == 1, f'Didn\'t find exactly 1 modification for event {action.eventId}.')
            
            if i == 0:
                self.check(modifs[0].depthTestFailed, f'Depth test did not fail for event {action.eventId}.')
            elif i == 1:
                self.check(modifs[0].stencilTestFailed, f'Stencil test did not fail for event {action.eventId}.')
            elif i == 2:
                self.check(modifs[0].scissorClipped, f'Scissor test did not fail for event {action.eventId}.')
            elif i == 3:
                self.check(modifs[0].backfaceCulled, f'Face culling did not fail for event {action.eventId}.')
            elif i == 4:
                self.check(modifs[0].sampleMasked, f'Sample masking did not fail for event {action.eventId}.')
            else:
                raise TestFailureException('Unexpected event.')

            rdtest.log.success(f'Test {action.eventId} completed.')
            i += 1
